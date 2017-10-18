#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

//#include <unistd.h>
#include <neat.h>

#define APP_VERSION "1.0.2"

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
  
  snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nUser-agent: pmtest\r\nConnection: close\r\n\r\n", "/", "celerway.com");

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
  //if (bytes_read > 0) {
  //  fwrite(buffer, sizeof(char), bytes_read, stdout);
  //}

  return NEAT_OK;
}

neat_error_code on_connected(struct neat_flow_operations *op)
{
  neat_error_code err = NEAT_OK;
  char buff[128];
  size_t buff_size = 128;
  err = neat_get_property(op->ctx, op->flow, "interface", buff, &buff_size);
  if (err == NEAT_OK) {
    fprintf(stderr, "INFO: on_connected invoked! interface: %s\n", buff);
  } else {
    fprintf(stderr, "INFO: on_connected invoked! WARINING Can't query interface porperty\n");
  }
  
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

/*const char *flow_property = "\
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
}";*/

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

const char *dst_host = "celerway.com";
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

int main(int argc, char *argv[])
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