#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "version.h"
#include "arg_parser.h"
#include "logger.h"

inline int min(int a, int b) { return a < b ? a : b; }
inline int max(int a, int b) { return a > b ? a : b; }

void print_usage(void)
{
  const char *usage = "\n"
    APP_NAME " [OPTIONS] HOST\n"
    "\n"
    "--port=PORT, -p PORT              Port nubmer to connect to, default 80\n"
    "--mode=MODE, -m MODE              Mode, possible values connect or echo\n"
    "                                  The default mode is connect\n"
    "--count=COUNT, -n COUNT           Number of pings to run, default 1\n"
    "--interval=INTERVAL, -i INTERVAL  Interval in seconds between pings, default 1\n"
    "--timeout=TIMEOUT, -t TIMEOUT     Application timeout in seconds, deault no timeout\n"
    "--bind=IFNAME, -b IFNAME          Bind interface name\n"
    "--verbose[=N], -v[vvv]             Verbosity level 0,1,2 or 3\n"
    "--help, -h                        Display this usage and exits\n"
    "--version, -V                     Display version number and exits\n";

  fprintf(stdout, "%s\n", usage);
}

void print_version(void)
{
  fprintf(stdout, "%s version %s\n", APP_NAME, APP_VERSION);
}

void parse_args(int argc, char *argv[], struct app_config *cfg)
{
  static struct option long_options[] =
  {
    {"port", required_argument, 0, 'p'},
    {"mode", required_argument, 0, 'm'},
    {"count", required_argument, 0, 'n'},
    {"interval", required_argument, 0, 'i'},
    {"timeout", required_argument, 0, 't'},
    {"bind", required_argument, 0, 'b'},
    {"verbose", optional_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {0, 0, 0, 0}
  };

  int option = 0;
  int option_index = 0;

  cfg->host = NULL;
  cfg->port = 80;
  cfg->mode = PING_MODE_CONNECT;
  cfg->count = 1;
  cfg->interval = 1;
  cfg->timeout = 0;
  cfg->verbose = 0;
  cfg->bind_ifname = NULL;
  
  while(1) {
    option = getopt_long(argc, argv, "p:m:n:i:t:b:vhV", long_options, &option_index);
    if (option == -1) {
      break;
    }

    switch(option) {
      case 'p':
        cfg->port = strtol(optarg, NULL, 10);
        break;
      case 'm':
        if (strcmp(optarg, "connect") == 0) {
          cfg->mode = PING_MODE_CONNECT;
        } else if (strcmp(optarg, "echo") == 0) {
          cfg->mode = PING_MODE_ECHO;
        } else {
          log_error("Invalid mode argument, expected connect or echo");
          exit(-1);
        }
        break;
      case 'n':
        cfg->count = strtol(optarg, NULL, 10);
        break;
      case 'i':
        cfg->interval = strtol(optarg, NULL, 10);
        break;
      case 't':
        cfg->timeout = strtol(optarg, NULL, 10);
        break;
      case 'b':
        cfg->bind_ifname = strdup(optarg);
        break;
      case 'v':
        cfg->verbose = optarg ? strtol(optarg, NULL, 10) : cfg->verbose + 1;
        cfg->verbose = max(min(cfg->verbose, 4), 0);
        break;
      case 'h':
        print_usage();
        exit(0);
      case 'V':
        print_version();
        exit(0);
        break;
      case '?':
        exit(-1);
      default:
        log_error("Fatal error while parsing command line arguments");
        exit(-1);

    }
  }

  if (optind < argc) {
    cfg->host = strdup(argv[optind++]);
  }

  if (optind < argc) {
    log_error("Too many program arguments");
    exit(-1);
  }

  if (!cfg->host) {
    log_error("Missing host argument");
    exit(-1);
  }
}
