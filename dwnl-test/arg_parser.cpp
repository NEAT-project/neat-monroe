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
    "--path=PATH, -x PATH              Requested URI path, the default value is '/'\n"
    "--count=COUNT, -n COUNT           Number of consecutive downloads to run, default 1\n"
    "--interval=INTERVAL, -i INTERVAL  Interval in seconds between each run, default 1\n"
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
    {"path", required_argument, 0, 'x'},
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

  memset(cfg->host, 0, DWNL_HOST_LEN);
  cfg->port = 80;
  memset(cfg->path, 0, DWNL_PATH_LEN);
  cfg->count = 1;
  cfg->interval = 1;
  cfg->timeout = 0;
  cfg->verbose = 0;
  memset(cfg->bind_ifname, 0, DWNL_BIND_IFNAME_LEN);
  
  while(1) {
    option = getopt_long(argc, argv, "p:x:n:i:t:b:vhV", long_options, &option_index);
    if (option == -1) {
      break;
    }

    switch(option) {
      case 'p':
        cfg->port = strtol(optarg, NULL, 10);
        break;
      case 'x':
        snprintf(cfg->path, DWNL_PATH_LEN, "%s", optarg);
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
        snprintf(cfg->bind_ifname, DWNL_BIND_IFNAME_LEN, "%s", optarg);
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
    snprintf(cfg->host, DWNL_HOST_LEN, "%s", argv[optind++]);
  }

  if (optind < argc) {
    log_error("Too many program arguments");
    exit(-1);
  }

  if (strlen(cfg->host) == 0) {
    log_error("Missing host argument");
    exit(-1);
  }

  if (strlen(cfg->path) == 0) {
    snprintf(cfg->path, DWNL_PATH_LEN, "/");
  }
}
