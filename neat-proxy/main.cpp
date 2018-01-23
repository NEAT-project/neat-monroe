#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

//#include <unistd.h>
#include <neat.h>

#define APP_VERSION "1.0.2"

static neat_error_code on_orig_dst_connected(struct neat_flow_operations *ops);
static neat_error_code on_tproxy_connected(struct neat_flow_operations *ops);
static neat_error_code on_readable(struct neat_flow_operations *ops);
static neat_error_code on_writable(struct neat_flow_operations *ops);
static neat_error_code on_all_written(struct neat_flow_operations *ops);
static neat_error_code on_close(struct neat_flow_operations *ops);
static neat_error_code on_aborted(struct neat_flow_operations *ops);
static neat_error_code on_timeout(struct neat_flow_operations *ops);
static neat_error_code on_tproxy_error(struct neat_flow_operations *ops);
static neat_error_code on_error(struct neat_flow_operations *ops);


static neat_error_code create_orig_dst_flow(struct neat_flow_operations *ops);

#define PEER_BUFFER_SIZE (10*1024)

struct flow_info
{
  // Pointer to the peer flow 
  struct neat_flow_operations *peer_ops;
  // Buffer for data to be send by this flow
  unsigned char *buffer;
  // Number of bytes in the buffer
  unsigned int num_of_bytes;
};

struct flow_info *
flow_info_alloc(struct neat_flow_operations *peer_ops)
{
  struct flow_info *fi = NULL;

  fprintf(stderr, "INFO: %s\n", __FUNCTION__);

  fi = (struct flow_info *)calloc(1, sizeof(flow_info));
  if (!fi) {
    fprintf(stderr, "ERROR: %s - failed to allocate memory for flow_info\n", __FUNCTION__);
    goto error;
  }
  
  fi->buffer = (unsigned char *)calloc(PEER_BUFFER_SIZE, sizeof(unsigned char));
  if (!fi->buffer) {
    fprintf(stderr, "ERROR: %s - failed to allocate memory for flow_info buffer\n", __FUNCTION__);
    goto error;
  }
  
  fi->num_of_bytes = 0;
  fi->peer_ops = peer_ops;

  return fi;

error:
  if (fi) {
    if (fi->buffer) {
      free(fi->buffer);
    }
    free(fi);
  }

  return NULL;
}

//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
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

void
log_addr(struct neat_flow_operations *ops, const char *prefix) 
{
  char local_ipaddr[32], remote_ipaddr[32];
  uint16_t local_port, remote_port;

  flow_get_addr(ops, local_ipaddr, 32, &local_port, 1);
  flow_get_addr(ops, remote_ipaddr, 32, &remote_port, 0);
  fprintf(stderr, "INFO: %s [local: %s:%d remote: %s:%d ]\n", prefix,
    local_ipaddr, local_port, remote_ipaddr, remote_port);
}

/*-------------------------------------------------------------------------------------------------------------------*/

// This is a new connection comming form tproxy side;
// create a flow to the original destination
static neat_error_code
on_tproxy_connected(struct neat_flow_operations *tproxy_ops)
{
  neat_error_code err = NEAT_OK;
  struct neat_ctx *ctx = tproxy_ops->ctx;
  struct neat_flow *orig_dst_flow = NULL;
  struct neat_flow_operations orig_dst_ops;
  struct flow_info *orig_dst_info = NULL;
  char orig_dst_ipaddr[32];
  uint16_t orig_dst_port;

  const char *properties = "{\
    \"transport\": {\"value\": \"TCP\", \"precedence\": 2},\
    \"multihoming\": {\"value\": false, \"precedence\": 2}\
    }";
  
  log_addr(tproxy_ops, __FUNCTION__);

  orig_dst_info = flow_info_alloc(tproxy_ops);
  if (!orig_dst_info) {
    fprintf(stderr, "ERROR: %s - orig_dst flow_info_alloc failed\n", __FUNCTION__);
    err = NEAT_ERROR_OUT_OF_MEMORY;
    goto error;
  }

  orig_dst_flow = neat_new_flow(ctx);
  if (!orig_dst_flow) {
    fprintf(stderr, "ERROR: %s - orig_dst neat_new_flow failed\n", __FUNCTION__);
    err = NEAT_ERROR_OUT_OF_MEMORY;
    goto error;
  }

  err = neat_set_property(ctx, orig_dst_flow, properties);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - orig_dst neat_set_property failed\n", __FUNCTION__);
    goto error;
  }

  memset(&orig_dst_ops, 0, sizeof(orig_dst_ops));
  orig_dst_ops.on_connected = on_orig_dst_connected;
  orig_dst_ops.on_error = on_error;
  orig_dst_ops.userData = orig_dst_info;
  err = neat_set_operations(ctx, orig_dst_flow, &orig_dst_ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - orig_dst neat_set_operations failed\n", __FUNCTION__);
    goto error;
  }

  flow_get_addr(tproxy_ops, orig_dst_ipaddr, 32, &orig_dst_port, 1);
  fprintf(stderr, "INFO: %s - connecting to %s:%d\n", __FUNCTION__, orig_dst_ipaddr, orig_dst_port);

  err = neat_open(ctx, orig_dst_flow, orig_dst_ipaddr, orig_dst_port, NULL, 0);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - orig_dst neat_open failed\n", __FUNCTION__);
    goto error;
  }

  // tproxy flow is now established, add on_close callback handling
  tproxy_ops->on_close = on_close;
  tproxy_ops->on_aborted = on_aborted;
  tproxy_ops->on_timeout = on_timeout;
  tproxy_ops->on_error = on_error;
  err = neat_set_operations(ctx, tproxy_ops->flow, tproxy_ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - tproxy neat_set_operations failed\n", __FUNCTION__);
    goto error;
  }

  return NEAT_OK;

error:

  //TODO add clean up

  return err;
}

// A connection to original destination has just been
// established; start data transmision by setting
// on_readable on both flows
static neat_error_code
on_orig_dst_connected(struct neat_flow_operations *orig_dst_ops)
{
  neat_error_code err = NEAT_OK;
  struct neat_flow_operations *tproxy_ops = NULL;
  struct flow_info *tproxy_info = NULL;

  log_addr(orig_dst_ops, __FUNCTION__);
  
  tproxy_info = flow_info_alloc(orig_dst_ops);
  if (!tproxy_info) {
    fprintf(stderr, "ERROR: %s - flow_info_alloc failed\n", __FUNCTION__);
    err = NEAT_ERROR_OUT_OF_MEMORY;
    goto error;
  }

  tproxy_ops = ((struct flow_info *)orig_dst_ops->userData)->peer_ops;
  tproxy_ops->userData = tproxy_info;
  tproxy_ops->on_readable = on_readable;
  err = neat_set_operations(tproxy_ops->ctx, tproxy_ops->flow, tproxy_ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - tproxy neat_set_operations failed\n", __FUNCTION__);
    goto error;
  }

  orig_dst_ops->on_readable = on_readable;
  orig_dst_ops->on_close = on_close;
  orig_dst_ops->on_aborted = on_aborted;
  orig_dst_ops->on_timeout = on_timeout;
  err = neat_set_operations(orig_dst_ops->ctx, orig_dst_ops->flow, orig_dst_ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - orig_dst neat_set_operations failed\n", __FUNCTION__);
    goto error;
  }

  return NEAT_OK;

error:

  //TODO add clean up

  return err;
}

static neat_error_code
on_readable(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = NULL;
  struct flow_info *peer_fi = NULL;

  log_addr(ops, __FUNCTION__);
//  proxy = 
//  peer = (struct flow_info *)((struct flow_info *)ops->userData)->ops->userData;

  fi = (struct flow_info *)ops->userData;
  peer_fi = (struct flow_info *)fi->peer_ops->userData;

  err = neat_read(ops->ctx, ops->flow, peer_fi->buffer, PEER_BUFFER_SIZE, &(peer_fi->num_of_bytes), NULL, 0);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: on_readable failed\n");
    goto error;
  }

  fprintf(stderr, "INFO: on_readable - received %d bytes\n", peer_fi->num_of_bytes);
  //if (bytes_read > 0) {
  //  buffer[bytes_read] = 0;
  //  fprintf(stderr, "%s\n", buffer);
  //}

  ops->on_readable = NULL;
  err = neat_set_operations(ops->ctx, ops->flow, ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - neat_set_operations failed\n", __FUNCTION__);
    goto error;
  }

  fi->peer_ops->on_writable = on_writable;
  err = neat_set_operations(fi->peer_ops->ctx, fi->peer_ops->flow, fi->peer_ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - neat_set_operations failed\n", __FUNCTION__);
    goto error;
  }

  return NEAT_OK;

error:
  return err;
}

static neat_error_code
on_writable(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = NULL;

  log_addr(ops, __FUNCTION__);

  fi = (struct flow_info *)ops->userData;
  err = neat_write(ops->ctx, ops->flow, fi->buffer, fi->num_of_bytes, NULL, 0);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - neat_write failed\n", __FUNCTION__);
    goto error;
  }

  ops->on_writable = NULL;
  ops->on_all_written = on_all_written;
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
on_all_written(struct neat_flow_operations *ops)
{
  neat_error_code err = NEAT_OK;
  struct flow_info *fi = NULL;
  
  log_addr(ops, __FUNCTION__);

  ops->on_all_written = NULL;
  err = neat_set_operations(ops->ctx, ops->flow, ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - neat_set_operations failed\n", __FUNCTION__);
    goto error;
  }

  fi = (struct flow_info *)ops->userData;
  fi->num_of_bytes = 0;
  fi->peer_ops->on_readable = on_readable;
  err = neat_set_operations(fi->peer_ops->ctx, fi->peer_ops->flow, fi->peer_ops);
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
  //fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  log_addr(ops, __FUNCTION__);
  return NEAT_OK;
}

static neat_error_code
on_timeout(struct neat_flow_operations *ops)
{
  struct flow_info *fi = NULL;

  //fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  log_addr(ops, __FUNCTION__);
  //neat_stop_event_loop(ops->ctx);
  neat_close(ops->ctx, ops->flow);

  // Close peer flow and free flow info memory
  fi = (struct flow_info *)ops->userData;
  if (fi) {
    neat_close(fi->peer_ops->ctx, fi->peer_ops->flow);
    free(fi->buffer);
    free(fi);
  }

  return NEAT_OK;
}

static neat_error_code
on_close(struct neat_flow_operations *ops)
{
  struct flow_info *fi = NULL;

  log_addr(ops, __FUNCTION__);

  // Close peer flow and free flow info memory
  fi = (struct flow_info *)ops->userData;
  if (fi) {
    neat_close(fi->peer_ops->ctx, fi->peer_ops->flow);
    free(fi->buffer);
    free(fi);
  }

  return NEAT_OK;
}

static neat_error_code
on_tproxy_error(struct neat_flow_operations *ops)
{
  fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  neat_stop_event_loop(ops->ctx);
  return NEAT_OK;
}

static neat_error_code
on_error(struct neat_flow_operations *ops)
{
  struct flow_info *fi = NULL;

  //fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  log_addr(ops, __FUNCTION__);
  //neat_stop_event_loop(ops->ctx);
  //neat_close(ops->ctx, ops->flow);

  // Close peer flow and free flow info memory
  fi = (struct flow_info *)ops->userData;
  if (fi) {
    neat_close(fi->peer_ops->ctx, fi->peer_ops->flow);
    free(fi->buffer);
    free(fi);
  }

  return NEAT_OK;
}

int main(int argc, char *argv[])
{
  neat_error_code err = NEAT_OK;
  struct neat_ctx *ctx = NULL;
  struct neat_flow *flow = NULL;
  struct neat_flow_operations ops;

  const char *properties = "{\
    \"transport\": {\"value\": \"TCP\", \"precedence\": 2},\
    \"tproxy\": {\"value\": true, \"precedence\": 2}\
  }";

  fprintf(stderr, "INFO: neat-proxy daemon started\n");

  ctx = neat_init_ctx();
  if (!ctx) {
    fprintf(stderr, "ERROR: neat_init_ctx failed!\n");
    goto cleanup;
  }

  neat_log_level(ctx, NEAT_LOG_DEBUG);

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
  ops.on_connected = on_tproxy_connected;
  ops.on_error = on_tproxy_error;
  neat_set_operations(ctx, flow, &ops);

  err = neat_accept(ctx, flow, 9876, NULL, 0);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: neat_new_flow failed");
    goto cleanup;
  }

  err = neat_start_event_loop(ctx, NEAT_RUN_DEFAULT);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: neat_start_event_loop failed");
    goto cleanup;
  }
  
cleanup:
  if (ctx) {
    neat_free_ctx(ctx);
  }

  fprintf(stderr, "INFO: neat-proxy daemon terminated\n");
  return 0;
}
