#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <neat.h>

#include "version.h"
#include "arg_parser.h"
#include "logger.h"

static neat_error_code on_connected_connect(struct neat_flow_operations *ops);
static neat_error_code on_connected_echo(struct neat_flow_operations *ops);

static neat_error_code on_readable(struct neat_flow_operations *ops);
static neat_error_code on_writable(struct neat_flow_operations *ops);
static neat_error_code on_all_written(struct neat_flow_operations *ops);
static neat_error_code on_close(struct neat_flow_operations *ops);
static neat_error_code on_aborted(struct neat_flow_operations *ops);
static neat_error_code on_timeout(struct neat_flow_operations *ops);
static neat_error_code on_error(struct neat_flow_operations *ops);

#define LOCAL_ADDR_LEN (32)

struct flow_info
{
  struct timespec ts1;
  struct timespec ts2;
  struct app_config cfg;
  int iter;
  struct timespec ts_app_start;
  char local_addr[LOCAL_ADDR_LEN];
  uint16_t local_port;
  neat_error_code err;
};

void print_rtt(struct flow_info *fi, int mode)
{
  clock_gettime(CLOCK_REALTIME, &fi->ts2);

  if (fi->ts2.tv_nsec < fi->ts1.tv_nsec) {
    fi->ts2.tv_nsec += 1000000000;
    fi->ts2.tv_sec--;
  }

  fi->iter += 1;

  fprintf(stdout, "neat-tcp-ping\t%d\t%s\t%s\t%d\t%s\t%ld.%09ld\n",
    fi->iter, fi->local_addr, fi->cfg.host, fi->cfg.port,
    mode == PING_MODE_ECHO ? "ECHO" : "CONN",
    (long)(fi->ts2.tv_sec - fi->ts1.tv_sec),
    fi->ts2.tv_nsec - fi->ts1.tv_nsec);
}

void
flow_get_addr(struct neat_flow_operations *ops, char *ipaddr_buf, int ipaddr_buf_len, uint16_t *port, int local)
{
  struct sockaddr* addrs = NULL;
  struct sockaddr_in* addr = NULL;
  int n = neat_getlpaddrs(ops->ctx, ops->flow, &addrs, local);
  if (n > 0) {
    addr = (struct sockaddr_in*)&addrs[0];
    snprintf(ipaddr_buf, ipaddr_buf_len, "%s", inet_ntoa(addr->sin_addr));
    *port = ntohs(addr->sin_port);
  } else {
    snprintf(ipaddr_buf, ipaddr_buf_len, "?");
    *port = 0;
  }

  if (addrs) {
    free(addrs);
  }
}

int check_app_timeout(struct flow_info *fi)
{
  struct timespec ts;

  if (fi->cfg.timeout > 0) {
    clock_gettime(CLOCK_REALTIME, &ts);
    if (ts.tv_sec - fi->ts_app_start.tv_sec > fi->cfg.timeout) {
      log_warning("App timeout reached");
      return 1;
    }
  }

  return 0;
}

static neat_error_code
tcp_ping_run_neat(neat_flow_operations_fx on_connected_fx, struct flow_info *fi)
{
  neat_error_code err = NEAT_OK;
  struct neat_ctx *ctx = NULL;
  struct neat_flow *flow = NULL;
  struct neat_flow_operations ops;

  const char *properties = "{\
    \"transport\": {\"value\": \"TCP\", \"precedence\": 2}\
  }";

  fi->err = NEAT_OK;

  ctx = neat_init_ctx();
  if (!ctx) {
    err = NEAT_ERROR_INTERNAL;
    log_error("neat_init_ctx failed");
    goto cleanup;
  }

  neat_log_level(ctx, fi->cfg.verbose);

  flow = neat_new_flow(ctx);
  if (!flow) {
    err = NEAT_ERROR_INTERNAL;
    log_error("neat_new_flow failed");
    goto cleanup;
  }

  err = neat_set_property(ctx, flow, properties);
  if (err) {
    log_error("neat_set_property failed");
    goto cleanup;
  }

  memset(&ops, 0, sizeof(ops));
  ops.on_connected = on_connected_fx;
  ops.on_close = on_close;
  ops.on_aborted = on_aborted;
  ops.on_timeout = on_timeout;
  ops.on_error = on_error;
  ops.userData = fi;
  err = neat_set_operations(ctx, flow, &ops);
  if (err) {
    log_error("neat_set_operations failed");
    goto cleanup;
  }

  err = neat_open(ctx, flow, fi->cfg.host, fi->cfg.port, NULL, 0);
  if (err) {
    log_error("neat_open failed");
    goto cleanup;
  }

  err = neat_start_event_loop(ctx, NEAT_RUN_DEFAULT);
  if (err) {
    log_error("neat_start_event_loop failed");
    goto cleanup;
  }

cleanup:
  if (ctx) {
    neat_free_ctx(ctx);
  }

  return (fi->err != NEAT_OK) ? fi->err : err;
}

static neat_error_code
tcp_ping_run_connect(struct flow_info *fi)
{
  neat_error_code err = NEAT_OK;

  fi->iter = 0;
  while(fi->iter < fi->cfg.count) {
    if (fi->iter > 0) {
      sleep(fi->cfg.interval);
    }

    clock_gettime(CLOCK_REALTIME, &fi->ts1);

    err = tcp_ping_run_neat(on_connected_connect, fi);
    if (err != NEAT_OK) {
      log_error("tcp_ping_run_neat failed");
      break;
    }

    if (check_app_timeout(fi)) {
      break;
    }
  }

  return err;
}

static neat_error_code
tcp_ping_run_echo(struct flow_info *fi)
{
  neat_error_code err = NEAT_OK;
  
  fi->iter = 0;
  clock_gettime(CLOCK_REALTIME, &fi->ts1);

  err = tcp_ping_run_neat(on_connected_echo, fi);
  if (err != NEAT_OK) {
    log_error("tcp_ping_run_neat failed");
  }

  return err;
}

/*-------------------------------------------------------------------------------------------------------------------*/

static neat_error_code
on_connected_connect(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = (struct flow_info *)ops->userData;

  log_debug(__FUNCTION__);

  flow_get_addr(ops, fi->local_addr, LOCAL_ADDR_LEN, &fi->local_port, 1);

  print_rtt(fi, PING_MODE_CONNECT);

  neat_close(ops->ctx, ops->flow);
  neat_stop_event_loop(ops->ctx);

  return NEAT_OK;
}


static neat_error_code
on_connected_echo(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = (struct flow_info *)ops->userData;

  log_debug(__FUNCTION__);

  flow_get_addr(ops, fi->local_addr, LOCAL_ADDR_LEN, &fi->local_port, 1);

  print_rtt(fi, PING_MODE_CONNECT);

  ops->on_writable = on_writable;
  err = neat_set_operations(ops->ctx, ops->flow, ops);
  if (err != NEAT_OK) {
    log_error("neat_set_operations failed");
    goto cleanup;
  }

cleanup:

  return err;
}

static neat_error_code
on_readable(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = (struct flow_info *)ops->userData;
  char buffer[128];
  uint32_t num_of_bytes = 0;

  log_debug(__FUNCTION__);

  err = neat_read(ops->ctx, ops->flow, (unsigned char*)buffer, sizeof(buffer), &num_of_bytes, NULL, 0);
  if (err != NEAT_OK) {
    log_error("neat_read failed");
    goto error;
  }

  print_rtt(fi, PING_MODE_ECHO);

  if (fi->iter >= fi->cfg.count || check_app_timeout(fi)) {
    neat_stop_event_loop(ops->ctx);
  } else {
    ops->on_readable = NULL;
    ops->on_writable = on_writable;
    err = neat_set_operations(ops->ctx, ops->flow, ops);
    if (err != NEAT_OK) {
      log_error("neat_set_operations failed");
      goto error;
    }
  }

error:

  return err;
}

static neat_error_code
on_writable(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = (struct flow_info *)ops->userData;
  char buffer[] = "12345678901234567890123456789012345678901234567890123456789012\n";

  log_debug(__FUNCTION__);

  sleep(fi->cfg.interval);

  clock_gettime(CLOCK_REALTIME, &fi->ts1);

  err = neat_write(ops->ctx, ops->flow, (unsigned char*)buffer, sizeof(buffer), NULL, 0);
  if (err != NEAT_OK) {
    log_error("neat_write failed");
    goto error;
  }

  ops->on_writable = NULL;
  ops->on_all_written = on_all_written;
  err = neat_set_operations(ops->ctx, ops->flow, ops);
  if (err != NEAT_OK) {
    log_error("neat_set_operations failed");
    goto error;
  }

  return NEAT_OK;

error:
  return err;
}

static neat_error_code
on_all_written(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;

  log_debug(__FUNCTION__);

  ops->on_all_written = NULL;
  ops->on_readable = on_readable;
  err = neat_set_operations(ops->ctx, ops->flow, ops);
  if (err != NEAT_OK) {
    log_error("neat_set_operations failed");
    goto error;
  }

  return NEAT_OK;

error:
  return err;
}

static neat_error_code
on_aborted(struct neat_flow_operations *ops)
{
  struct flow_info *fi = (struct flow_info *)ops->userData;

  log_error(__FUNCTION__);

  fi->err = NEAT_ERROR_INTERNAL;
  neat_stop_event_loop(ops->ctx);
  return NEAT_OK;
}

static neat_error_code
on_timeout(struct neat_flow_operations *ops)
{
  struct flow_info *fi = (struct flow_info *)ops->userData;

  log_error(__FUNCTION__);

  fi->err = NEAT_ERROR_INTERNAL;
  neat_stop_event_loop(ops->ctx);
  return NEAT_OK;
}

static neat_error_code
on_close(struct neat_flow_operations *ops)
{
  log_debug(__FUNCTION__);

  return NEAT_OK;
}

static neat_error_code
on_error(struct neat_flow_operations *ops)
{
  struct flow_info *fi = (struct flow_info *)ops->userData;

  log_error(__FUNCTION__);

  fi->err = NEAT_ERROR_INTERNAL;
  neat_stop_event_loop(ops->ctx);
  return NEAT_OK;
}

int main(int argc, char *argv[])
{
  neat_error_code err = NEAT_OK;
  struct flow_info fi;
  memset(&fi, 0, sizeof(struct flow_info));

  parse_args(argc, argv, &fi.cfg);

  log_level(fi.cfg.verbose);
  log_info("%s v%s started", APP_NAME, APP_VERSION);
  log_info("Host %s", fi.cfg.host);
  log_info("Port %d", fi.cfg.port);
  log_info("Mode %s", fi.cfg.mode == PING_MODE_ECHO ? "ECHO" : "CONN");
  log_info("Count %d", fi.cfg.count);
  log_info("Interval %d", fi.cfg.interval);
  log_info("Timeout %d", fi.cfg.timeout);
  log_info("Bind %s", fi.cfg.bind_ifname);
  log_info("Verbose %d", fi.cfg.verbose);

  clock_gettime(CLOCK_REALTIME, &fi.ts_app_start);

  if (fi.cfg.mode == PING_MODE_CONNECT) {
    log_debug("PING_MODE_CONNECT");
    err = tcp_ping_run_connect(&fi);
    if (err != NEAT_OK) {
      log_error("tcp_ping_run_connect failed");
    }
  }
  else {
    log_debug("PING_MODE_ECHO");
    err = tcp_ping_run_echo(&fi);
    if (err != NEAT_OK) {
      log_error("tcp_ping_run_echo failed");
    }
  }

  log_info("%s v%s terminated", APP_NAME, APP_VERSION);
  return err == NEAT_OK ? 0 : -1;
}
