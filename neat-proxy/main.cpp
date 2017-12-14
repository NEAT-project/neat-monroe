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
static neat_error_code on_error(struct neat_flow_operations *ops);


static neat_error_code create_orig_dst_flow(struct neat_flow_operations *ops);




/*

neat_error_code on_error(struct neat_flow_operations *op)
{
  fprintf(stderr, "ERROR: on_error invoked!\n");
  neat_stop_event_loop(op->ctx);
  return NEAT_OK;
}



neat_error_code on_writable(struct neat_flow_operations *op)
{
  static char request[512];
  neat_error_code err = NEAT_OK;
  
  snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nUser-agent: curl\r\nConnection: close\r\n\r\n", "/", "www.multipath-tcp.org");

  fprintf(stderr, "INFO: on_writable\n");

  err = neat_write(op->ctx, op->flow, (const unsigned char *)request, strlen(request), NULL, 0);
  if (err != NEAT_OK) {
    return on_error(op);
  }
  
  fprintf(stderr, "INFO: on_writable - sending request: \n%s\n", request);

  op->on_writable = NULL;
  neat_set_operations(op->ctx, op->flow, op);
  
  return NEAT_OK;
}

unsigned char *buffer = NULL;

neat_error_code on_readable(struct neat_flow_operations *op)
{
  fprintf(stderr, "INFO: on_readable\n");
  neat_error_code err = NEAT_OK;

  uint32_t bytes_read = 0;

  err = neat_read(op->ctx, op->flow, buffer, 32*1024*1024, &bytes_read, NULL, 0);
  if (err == NEAT_ERROR_WOULD_BLOCK) {
    fprintf(stderr, "WARNING: on_readable would block\n");
    return NEAT_OK;
  } else if (err != NEAT_OK) {
      return on_error(op);
  }

  fprintf(stderr, "INFO: on_readable - received %d bytes\n", bytes_read);
  if (bytes_read > 0) {
    buffer[bytes_read] = 0;
    fprintf(stderr, "%s\n", buffer);

//    fwrite(buffer, sizeof(char), bytes_read, stdout);
  }

  return NEAT_OK;
}

neat_error_code on_connected(struct neat_flow_operations *op)
{
  neat_error_code err = NEAT_OK;
  char buff[128];
  size_t buff_size = 128;
  //err = neat_get_property(op->ctx, op->flow, "interface", buff, &buff_size);
  //if (err == NEAT_OK) {
  //  fprintf(stderr, "INFO: on_connected invoked! interface: %s\n", buff);
  //} else {
  //  fprintf(stderr, "INFO: on_connected invoked! WARINING Can't query interface porperty\n");
  //}
  fprintf(stderr, "INFO: on_connected invoked!\n");
  
  op->on_readable = on_readable;
  op->on_writable = on_writable;
  neat_set_operations(op->ctx, op->flow, op);

  return NEAT_OK;
}

neat_error_code on_close(struct neat_flow_operations *op)
{
  fprintf(stderr, "INFO: on_close invoked!\n");
  neat_stop_event_loop(op->ctx);
  return NEAT_OK;
}

*const char *flow_property = "\
{\
    \"transport\": [\
        {\
            \"value\": \"SCTP\",\
            \"precedence\": 1\
        },\
        {\
            \"value\": \"TCP\",\
            \"precedence\": 1\
        }\
    ],\
    \"multihoming\": {\
        \"value\": true,\
        \"precedence\": 1\
    }\
}";*

const char *flow_property = "\
{\
    \"transport\": [\
        {\
            \"value\": \"TCP\",\
            \"precedence\": 1\
        }\
    ],\
    \"multihoming\": {\
        \"value\": true,\
        \"precedence\": 1\
    }\
}";

//const char *dst_host = "celerway.com";
const char *dst_host = "www.multipath-tcp.org";
const uint16_t dst_port = 80;

int run(const char *properties)
{

  buffer = (unsigned char*)malloc(32*1024*1024);

  printf("Hello world 5!\n");

  neat_error_code err = NEAT_OK;
  struct neat_ctx *ctx = NULL;
  struct neat_flow *flow = NULL;
  struct neat_flow_operations ops;

  memset(&ops, 0, sizeof(ops));

  ctx = neat_init_ctx();
  if (ctx == NULL) {
    fprintf(stderr, "ERROR: neat_init_ctx failed!\n");
  }

  neat_log_level(ctx, NEAT_LOG_DEBUG);

  flow = neat_new_flow(ctx);
  if (flow == NULL) {
    fprintf(stderr, "ERROR: neat_new_flow failed");
    goto cleanup;
  }

  err = neat_set_property(ctx, flow, properties);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: neat_set_property failed");
    goto cleanup;
  }

  ops.on_connected = on_connected;
  ops.on_error = on_error;
  ops.on_close = on_close;
  err = neat_set_operations(ctx, flow, &ops);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: neat_set_operations failed");
    goto cleanup;
  }

  err = neat_open(ctx, flow, dst_host, dst_port, NULL, 0);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: neat_open failed");
    goto cleanup;
  }

  err = neat_start_event_loop(ctx, NEAT_RUN_DEFAULT);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: neat_start_event_loop failed");
    goto cleanup;
  }

cleanup:

  if (ctx != NULL) {
    neat_free_ctx(ctx);
  }

  printf("Good bye world!\n");
  return 0;
}

const char *default_properties = "\
{\
    \"transport\": [\
        {\
            \"value\": \"TCP\",\
            \"precedence\": 1\
        }\
    ],\
    \"multihoming\": {\
        \"value\": true,\
        \"precedence\": 1\
    }\
}";

char *read_file(const char *file_name)
{
  FILE *file = NULL;
  char *buff = NULL;
  long len = 0;

  file = fopen(file_name, "r");
  if (file) {
    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);
    buff = (char *)malloc(len);
    if (buff) {
      fread(buff, 1, len, file);
    }
    fclose(file);
  }

  return buff;
}

int mainA(int argc, char *argv[])
{
  int res = 0;
  char *prop_file = NULL;
  char *properties = NULL;

  const char *help_string =
  " [--property-file|-f FILE]\n"
  "                  [--help|-h] [--versoin|-v]\n"
  "\n"
  "  --property-file|-f FILE\n"
  "        JSON file with required flow properties\n"
  "  --help|-h\n"
  "        Prints this information and exits\n"
  "  --versoin|-v\n"
  "        Prints program version and exists\n";

  static struct option long_options[] = {
    { "property-file", required_argument, 0, 'f' },
    { "help", no_argument, 0, 'h' },
    { "version", no_argument, 0, 'v'},
    { 0, 0, 0, 0 }
  };

  while (true) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "f:hv",
                        long_options, &option_index);
    if (c == -1) {
      break;
    }

    switch (c) {
      case 'f':
        prop_file = strdup(optarg);
        break;

      case 'h':
        fprintf(stderr, "\n%s%s\n", argv[0], help_string);
        return 0;

      case 'v':
        fprintf(stderr, "\n%s %s\n", argv[0], APP_VERSION);
        return 0;

      case '?':
      default:
        return -1;
    }
  }

  if (optind < argc) {
    fprintf(stderr, "%s: too many arguments", argv[0]);
    return -1;
  }

  printf("prop_file: %s\n", prop_file);

  if (prop_file != NULL) {
    properties = read_file(prop_file);
  } else {
    properties = strdup(default_properties);
  }

  printf("properties:\n%s\n", properties);
  res = run(properties);

cleanup:
  if (prop_file != NULL) {
    free(prop_file);
  }

  if (properties != NULL) {
    free(properties);
  }

  return res;
}

*-------------------------------------------------------------------------------------------------------------------*/

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
flow_get_addr(struct neat_flow_operations *ops, char *addr_buf, int addr_buf_len, int local)
{
  struct sockaddr* addrs = NULL;
  struct sockaddr_in* addr = NULL;
  int n = neat_getlpaddrs(ops->ctx, ops->flow, &addrs, local);
  if (n > 0) {
    addr = (struct sockaddr_in*)&addrs[0];
    snprintf(addr_buf, addr_buf_len, "%s:%d", inet_ntoa(addr->sin_addr), addr->sin_port);
  } else {
    snprintf(addr_buf, addr_buf_len, "?:?");
  }

  if (addrs) {
    free(addrs);
  }
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

  char local_addr[32], remote_addr[32];

  const char *properties = "{\
    \"transport\": {\"value\": \"TCP\", \"precedence\": 2},\
    \"multihoming\": {\"value\": false, \"precedence\": 2}\
    }";
  
  flow_get_addr(tproxy_ops, local_addr, 32, 1);
  flow_get_addr(tproxy_ops, remote_addr, 32, 0);
  fprintf(stderr, "INFO: %s [ %s -> %s ]\n", __FUNCTION__, remote_addr, local_addr);

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

  err = neat_open(ctx, orig_dst_flow, "wp.pl", 80, NULL, 0);
  if (err != NEAT_OK) {
    fprintf(stderr, "ERROR: %s - orig_dst neat_open failed\n", __FUNCTION__);
    goto error;
  }

  // tproxy flow is now established, add on_close callback handling
  tproxy_ops->on_close = on_close;
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
  char local_addr[32], remote_addr[32];

  flow_get_addr(orig_dst_ops, local_addr, 32, 1);
  flow_get_addr(orig_dst_ops, remote_addr, 32, 0);
  fprintf(stderr, "INFO: %s [ %s -> %s ]\n", __FUNCTION__, local_addr, remote_addr);

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

  char local_addr[32], remote_addr[32];

  flow_get_addr(ops, local_addr, 32, 1);
  flow_get_addr(ops, remote_addr, 32, 0);
  fprintf(stderr, "INFO: %s [ %s -> %s ]\n", __FUNCTION__, remote_addr, local_addr);
  
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

  char local_addr[32], remote_addr[32];

  flow_get_addr(ops, local_addr, 32, 1);
  flow_get_addr(ops, remote_addr, 32, 0);
  fprintf(stderr, "INFO: %s [ %s -> %s ]\n", __FUNCTION__, local_addr, remote_addr);
 
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
  
  char local_addr[32], remote_addr[32];

  flow_get_addr(ops, local_addr, 32, 1);
  flow_get_addr(ops, remote_addr, 32, 0);
  fprintf(stderr, "INFO: %s [ %s -> %s ]\n", __FUNCTION__, local_addr, remote_addr);

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
  fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  return NEAT_OK;
}

static neat_error_code
on_timeout(struct neat_flow_operations *ops)
{
  fprintf(stderr, "INFO: %s\n", __FUNCTION__);
  return NEAT_OK;
}

static neat_error_code
on_close(struct neat_flow_operations *ops)
{
  char /*local_addr[32],*/ remote_addr[32];

  //flow_get_addr(orig_dst_ops, local_addr, 32, 1);
  flow_get_addr(ops, remote_addr, 32, 0);
  fprintf(stderr, "INFO: %s [ %s ]\n", __FUNCTION__, remote_addr);
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
  struct neat_ctx *ctx = NULL;
  struct neat_flow *flow = NULL;
  struct neat_flow_operations ops;

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

  memset(&ops, 0, sizeof(ops));
  ops.on_connected = on_tproxy_connected;
  ops.on_error = on_error;
  neat_set_operations(ctx, flow, &ops);

  err = neat_accept(ctx, flow, 5000, NULL, 0);
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

