#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "version.h"
#include "arg_parser.h"

inline int min(int a, int b) { return a < b ? a : b; }
inline int max(int a, int b) { return a > b ? a : b; }

void print_usage(void)
{
  const char *usage = "\n"
    APP_NAME " [OPTIONS] HOST\n"
    "\n"
    "--port=PORT, -p PORT             Port nubmer to connect to, default 80\n"
    "--path=PATH, -x PATH             Requested URI path, the default value is '/'\n"
        "--count=COUNT, -n COUNT      Number of consecutive downloads to run\n"
    "--interval=INTERVAL -i INTERVAL  Interval in seconds between each run\n"
    "--verbose[=N], -v[vvv]           Verbosity level 1,2,4 or 4\n"
    "--help, -h                       Display this usage and exits\n"
    "--version, -V                    Display version number and exits\n";

  fprintf(stderr, "%s\n", usage);
}

void print_version(void)
{
  fprintf(stderr, "%s version %s\n", APP_NAME, APP_VERSION);
}

void parse_args(int argc, char *argv[], struct app_config *cfg)
{
  static struct option long_options[] =
  {
    {"port", required_argument, 0, 'p'},
    {"path", required_argument, 0, 'x'},
    {"count", required_argument, 0, 'n'},
    {"interval", required_argument, 0, 'i'},
    {"verbose", optional_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {0, 0, 0, 0}
  };

  int option = 0;
  int option_index = 0;

  cfg->host = NULL;
  cfg->port = 80;
  cfg->path = NULL;
  cfg->count = 10;
  cfg->interval = 1;
  cfg->verbose = 0;
  
  while(1) {
    option = getopt_long(argc, argv, "p:x:n:i:vhV", long_options, &option_index);
    if (option == -1) {
      break;
    }

    switch(option) {
      case 'p':
        cfg->port = strtol(optarg, NULL, 10);
        fprintf(stderr, "INFO: port %d\n", cfg->port);
        break;
      case 'x':
        if (cfg->path == NULL) {
          cfg->path = strdup(optarg);
        } else {
          fprintf(stderr, "ERROR: Path argument specified multiple times\n");
          exit(-1);
        }
        break;
      case 'n':
        cfg->count = strtol(optarg, NULL, 10);
        break;
      case 'i':
        cfg->interval = strtol(optarg, NULL, 10);
        break;
      case 'v':
        cfg->verbose += optarg ? strtol(optarg, NULL, 10) : 1;
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
        fprintf(stderr, "ERROR: Fatal error while parsing command line arguments\n");
        exit(-1);

    }
  }

  if (optind < argc) {
    cfg->host = strdup(argv[optind++]);
  }

  if (optind < argc) {
    fprintf(stderr, "ERROR: Too many program arguments\n");
    exit(-1);
  }

  if (!cfg->host) {
    fprintf(stderr, "ERROR: Missing host argument\n");
    exit(-1);
  }

  fprintf(stderr, "INFO: Host %s\n", cfg->host);
  fprintf(stderr, "INFO: Port %d\n", cfg->port);
  fprintf(stderr, "INFO: Path %s\n", cfg->path);
  fprintf(stderr, "INFO: Count %d\n", cfg->count);
  fprintf(stderr, "INFO: Interval %d\n", cfg->interval);
  fprintf(stderr, "INFO: Verbose %d\n", cfg->verbose);
}
