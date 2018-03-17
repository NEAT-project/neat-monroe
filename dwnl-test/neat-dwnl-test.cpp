#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <neat.h>

#include "version.h"
#include "arg_parser.h"
#include "logger.h"

static neat_error_code on_connected(struct neat_flow_operations *ops);
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
  struct timespec tsinit;
  struct timespec ts2;
  struct app_config cfg;
  int iter;
  struct timespec ts_app_start;
  int timeout_fired;
  int len;
  char local_addr[LOCAL_ADDR_LEN];
  uint16_t local_port;
  neat_error_code err;
};

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
      fi->timeout_fired = 1;
      log_warning("App timeout reached");
      return 1;
    }
  }

  return 0;
}

static neat_error_code
dwnl_test_open_flow(struct neat_ctx *ctx, struct flow_info *fi)
{
  neat_error_code err = NEAT_OK;
  struct neat_flow *flow = NULL;
  struct neat_flow_operations ops;

  const char *properties = "{\
    \"transport\": {\"value\": \"TCP\", \"precedence\": 2}\
  }";

  flow = neat_new_flow(ctx);
  if (!flow) {
    err = NEAT_ERROR_INTERNAL;
    log_error("neat_new_flow failed");
    goto cleanup;
  }

  err = neat_set_property(ctx, flow, properties);
  if (err != NEAT_OK) {
    log_error("neat_set_property failed");
    goto cleanup;
  }

  memset(&ops, 0, sizeof(ops));
  ops.on_connected = on_connected;
  ops.on_close = on_close;
  ops.on_aborted = on_aborted;
  ops.on_timeout = on_timeout;
  ops.on_error = on_error;
  ops.userData = fi;
  err = neat_set_operations(ctx, flow, &ops);
  if (err != NEAT_OK) {
    log_error("neat_set_operations failed");
    goto cleanup;
  }

  err = neat_open(ctx, flow, fi->cfg.host, fi->cfg.port, NULL, 0);
  if (err != NEAT_OK) {
    log_error("neat_open failed");
    goto cleanup;
  }

cleanup:
  return err;
}

static neat_error_code
dwnl_test_run(struct flow_info *fi)
{
  neat_error_code err = NEAT_OK;
  struct neat_ctx *ctx = NULL;

  fi->err = NEAT_OK;
  clock_gettime(CLOCK_REALTIME, &fi->ts1);

  ctx = neat_init_ctx();
  if (!ctx) {
    err = NEAT_ERROR_INTERNAL;
    log_error("neat_init_ctx failed");
    goto cleanup;
  }

  neat_log_level(ctx, fi->cfg.verbose);

  err = dwnl_test_open_flow(ctx, fi);
  if (err != NEAT_OK) {
    log_error("dwnl_test_open_flow failed");
    goto cleanup;
  }

  err = neat_start_event_loop(ctx, NEAT_RUN_DEFAULT);
  if (err != NEAT_OK) {
    log_error("neat_start_event_loop failed");
    goto cleanup;
  }

  clock_gettime(CLOCK_REALTIME, &fi->ts2);

cleanup:
  if (ctx) {
    neat_free_ctx(ctx);
  }

  return (fi->err != NEAT_OK) ? fi->err : err;
}

/*-------------------------------------------------------------------------------------------------------------------*/

static neat_error_code
on_connected(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = (struct flow_info *)ops->userData;

  log_debug(__FUNCTION__);

  flow_get_addr(ops, fi->local_addr, LOCAL_ADDR_LEN, &fi->local_port, 1);

  ops->on_writable = on_writable;
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
on_readable(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = (struct flow_info *)ops->userData;
  char buffer[2048];
  uint32_t num_of_bytes = 0;

  log_debug(__FUNCTION__);

  err = neat_read(ops->ctx, ops->flow, (unsigned char*)buffer, sizeof(buffer), &num_of_bytes, NULL, 0);
  if (err != NEAT_OK) {
    log_error("neat_read failed");
    goto error;
  }

  fi->len += num_of_bytes;

  if (check_app_timeout(fi)) {
    neat_close(ops->ctx, ops->flow);
    neat_stop_event_loop(ops->ctx);
  }

  return NEAT_OK;

error:
  return err;
}

static neat_error_code
on_writable(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = (struct flow_info *)ops->userData;
  char buffer[256];

  log_debug(__FUNCTION__);

  clock_gettime(CLOCK_REALTIME, &fi->tsinit);

  snprintf(buffer, sizeof(buffer),
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "User-agent: dwnl-test\r\n"
    "Connection: close\r\n"
    "\r\n",
    fi->cfg.path, fi->cfg.host);

  err = neat_write(ops->ctx, ops->flow, (unsigned char*)buffer, sizeof(buffer), NULL, 0);
  if (err != NEAT_OK) {
    log_error("neat_write failed");
    goto error;
  }

  fi->len = 0;

  ops->on_writable = NULL;
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

  neat_stop_event_loop(ops->ctx);
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
  log_info("Path %s", fi.cfg.path);
  log_info("Count %d", fi.cfg.count);
  log_info("Interval %d", fi.cfg.interval);
  log_info("Timeout %d", fi.cfg.timeout);
  log_info("Bind %s", fi.cfg.bind_ifname);
  log_info("Verbose %d", fi.cfg.verbose);

  clock_gettime(CLOCK_REALTIME, &fi.ts_app_start);

  fi.iter = 0;
  fi.timeout_fired = 0;
  while(fi.iter < fi.cfg.count && !fi.timeout_fired) {
    if (fi.iter > 0) {
      sleep(fi.cfg.interval);
      if (check_app_timeout(&fi)) {
          break;
      }
    }

    fi.iter += 1;
    err = dwnl_test_run(&fi);
    if (err != NEAT_OK) {
      log_error("dwnl_test_run failed");
      break;
    }

    if (fi.tsinit.tv_nsec < fi.ts1.tv_nsec) {
      fi.tsinit.tv_nsec += 1000000000;
      fi.tsinit.tv_sec--;
    }
    if (fi.ts2.tv_nsec < fi.ts1.tv_nsec) {
      fi.ts2.tv_nsec += 1000000000;
      fi.ts2.tv_sec--;
    }
    fprintf(stdout, "neat-dwnl-test\t%d\t%s\t%s\t%d\t%s\t%d\t%ld.%09ld\t%ld.%09ld\n",
      fi.iter, fi.local_addr, fi.cfg.host, fi.cfg.port, fi.cfg.path, fi.len,
      (long)(fi.tsinit.tv_sec - fi.ts1.tv_sec),
      fi.tsinit.tv_nsec - fi.ts1.tv_nsec,
      (long)(fi.ts2.tv_sec - fi.ts1.tv_sec),
      fi.ts2.tv_nsec - fi.ts1.tv_nsec);
  }

  log_info("%s v%s terminated", APP_NAME, APP_VERSION);
  return err == NEAT_OK ? 0 : -1;
}
