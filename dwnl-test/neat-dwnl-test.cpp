#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <neat.h>

#include "version.h"
#include "arg_parser.h"

static neat_error_code on_connected(struct neat_flow_operations *ops);
static neat_error_code on_readable(struct neat_flow_operations *ops);
static neat_error_code on_writable(struct neat_flow_operations *ops);
static neat_error_code on_all_written(struct neat_flow_operations *ops);
static neat_error_code on_close(struct neat_flow_operations *ops);
static neat_error_code on_aborted(struct neat_flow_operations *ops);
static neat_error_code on_timeout(struct neat_flow_operations *ops);
static neat_error_code on_error(struct neat_flow_operations *ops);

struct flow_info
{
  struct timespec ts1;
  struct timespec tsinit;
  struct timespec ts2;
  struct app_config cfg;
  int iter;
  int len;
};

static neat_error_code
dwnl_test_open_flow(struct neat_ctx *ctx, struct flow_info *fi)
{
  neat_error_code err = NEAT_OK;
  struct neat_flow *flow = NULL;
  struct neat_flow_operations ops;

  const char *properties = "{\
    \"transport\": {\"value\": \"TCP\", \"precedence\": 2}\
  }";

  fprintf(stderr, "INFO: %s\n", __FUNCTION__);

  flow = neat_new_flow(ctx);
  if (!flow) {
    fprintf(stderr, "ERROR: neat_new_flow failed");
    goto cleanup;
  }

  err = neat_set_property(ctx, flow, properties);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - neat_set_property failed\n", __FUNCTION__);
    goto cleanup;
  }

  memset(&ops, 0, sizeof(ops));
  ops.on_connected = on_connected;
  ops.on_close = on_close;
  ops.on_aborted = on_aborted;
  ops.on_timeout = on_timeout;
  ops.on_error = on_error;
  ops.userData = fi;
  neat_set_operations(ctx, flow, &ops);

  err = neat_open(ctx, flow, fi->cfg.host, fi->cfg.port, NULL, 0);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - neat_open failed\n", __FUNCTION__);
    goto cleanup;
  }

  return NEAT_OK;

cleanup:
  return err;
}

static neat_error_code
dwnl_test_run(struct flow_info *fi)
{
  neat_error_code err = NEAT_OK;
  struct neat_ctx *ctx = NULL;

  clock_gettime(CLOCK_REALTIME, &fi->ts1);

  ctx = neat_init_ctx();
  if (!ctx) {
    fprintf(stderr, "ERROR: neat_init_ctx failed!\n");
    goto cleanup;
  }

  neat_log_level(ctx, fi->cfg.verbose);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: neat_start_event_loop failed");
    goto cleanup;
  }

  err = dwnl_test_open_flow(ctx, fi);

  err = neat_start_event_loop(ctx, NEAT_RUN_DEFAULT);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: neat_start_event_loop failed");
    goto cleanup;
  }

  clock_gettime(CLOCK_REALTIME, &fi->ts2);

cleanup:
  if (ctx) {
    neat_free_ctx(ctx);
  }

  return err;
}

/*-------------------------------------------------------------------------------------------------------------------*/

static neat_error_code
on_connected(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = (struct flow_info *)ops->userData;

  fprintf(stderr, "INFO: %s\n", __FUNCTION__);

  ops->on_writable = on_writable;
  err = neat_set_operations(ops->ctx, ops->flow, ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - neat_set_operations failed\n", __FUNCTION__);
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
  
  fprintf(stderr, "INFO: %s\n", __FUNCTION__);

  err = neat_read(ops->ctx, ops->flow, (unsigned char*)buffer, sizeof(buffer), &num_of_bytes, NULL, 0);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - neat_read\n", __FUNCTION__);
    goto error;
  }

  fi->len += num_of_bytes;
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

  fprintf(stderr, "INFO: %s\n", __FUNCTION__);

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
    fprintf(stderr, "ERROR: %s - neat_write failed\n", __FUNCTION__);
    goto error;
  }

  fi->len = 0;

  ops->on_writable = NULL;
  ops->on_readable = on_readable;
  err = neat_set_operations(ops->ctx, ops->flow, ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - neat_set_operations failed\n", __FUNCTION__);
    goto error;
  }

  return NEAT_OK;

error:
  return err;
}

static neat_error_code
on_aborted(struct neat_flow_operations *ops)
{
  fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  neat_stop_event_loop(ops->ctx);
  return NEAT_OK;
}

static neat_error_code
on_timeout(struct neat_flow_operations *ops)
{
  fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  neat_stop_event_loop(ops->ctx);
  return NEAT_OK;
}

static neat_error_code
on_close(struct neat_flow_operations *ops)
{
  struct flow_info *fi = (struct flow_info *)ops->userData;
  fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  neat_stop_event_loop(ops->ctx);
  return NEAT_OK;
}

static neat_error_code
on_error(struct neat_flow_operations *ops)
{
  fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  neat_stop_event_loop(ops->ctx);
  return NEAT_OK;
}

int main(int argc, char *argv[])
{
  neat_error_code err = NEAT_OK;
  struct flow_info fi;
  fprintf(stderr, "INFO: %s started\n", APP_NAME);

  parse_args(argc, argv, &fi.cfg);

  fi.iter = 0;
  while(fi.iter < fi.cfg.count) {
    fi.iter += 1;
    err = dwnl_test_run(&fi);
    if (err != NEAT_OK) {
      fprintf(stderr, "ERROR: %s - dwnl_test_run failed\n", __FUNCTION__);
      goto cleanup;
    }

    if (fi.tsinit.tv_nsec < fi.ts1.tv_nsec) {
      fi.tsinit.tv_nsec += 1000000000;
      fi.tsinit.tv_sec--;
    }
    if (fi.ts2.tv_nsec < fi.ts1.tv_nsec) {
      fi.ts2.tv_nsec += 1000000000;
      fi.ts2.tv_sec--;
    }
    fprintf(stdout, "neat-dwnl-test\t%d\t%s\t%d\t%s\t%d\t%ld.%09ld\t%ld.%09ld\n",
      fi.iter, fi.cfg.host, fi.cfg.port, fi.cfg.path, fi.len,
      (long)(fi.tsinit.tv_sec - fi.ts1.tv_sec),
      fi.tsinit.tv_nsec - fi.ts1.tv_nsec,
      (long)(fi.ts2.tv_sec - fi.ts1.tv_sec),
      fi.ts2.tv_nsec - fi.ts1.tv_nsec);

    sleep(fi.cfg.interval);
  }

cleanup:
  if (fi.cfg.host) {
    free(fi.cfg.host);
  }

  if (fi.cfg.path) {
    free(fi.cfg.path);
  }

  fprintf(stderr, "INFO: %s terminated\n", APP_NAME);
  return err == NEAT_OK ? 0 : -1;
}
