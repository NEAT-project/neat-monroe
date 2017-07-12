#include <iostream>
#include <getopt.h>
#include "mqdaemon.h"
#include "neat_writer.h"

#ifndef APP_VERSION
  #define APP_VERSION "0.0.0"
#endif

#define ZMQ_TOPIC "MONROE.META.DEVICE.MODEM"
#define ZMQ_ADDR "tcp://172.17.0.1:5556"

const char *help_string =
  " [--cib-socket|-s SOCKET] [--cib-prefix|-p PREFIX [--cib-extension|-e EXTENSION] ]\n"
  "                  [--help|-h] [--versoin|-v]\n"
  "\n"
  "  --cib-socket|-s SOCKET\n"
  "        Unix socket for communication with PM, e.g '/var/run/neat/pm_socket'\n"
  "  --cib-prefix|-p PREFIX\n"
  "        CIB file path and prefix, e.g. '/var/run/neat/cib/'\n"
  "  --cib-extension|-e EXTENSION]\n"
  "        CIB file extension, Default '.cib'\n"
  "  --help|-h\n"
  "        Prints this information and exits\n"
  "  --versoin|-v\n"
  "        Prints program version and exists\n";

int main(int argc, char **argv)
{
  std::string cib_socket;
  std::string cib_prefix;
  std::string cib_extension = ".cib";

  static struct option long_options[] = {
    { "cib-socket", required_argument, 0, 's' },
    { "cib-prefix", required_argument, 0, 'p' },
    { "cib-extension", required_argument, 0, 'e' },
    { "help", no_argument, 0, 'h' },
    { "version", no_argument, 0, 'v'},
    { 0, 0, 0, 0 }
  };

  while (true) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "s:p:e:hv",
                        long_options, &option_index);
    if (c == -1) {
      break;
    }

    switch (c) {
      case 's':
        cib_socket = optarg;
        break;

      case 'p':
        cib_prefix = optarg;
        break;

      case 'e':
        cib_extension = optarg;
        break;

      case 'h':
        std::cerr << std::endl << argv[0] << help_string << std::endl;
        return 0;

      case 'v':
        std::cerr << std::endl << argv[0] << " " << APP_VERSION << std::endl << std::endl;
        return 0;

      case '?':
      default:
        return -1;
    }
  }

  if (optind < argc) {
    std::cerr << argv[0] << ": too many arguments" << std::endl;
    return -1;
  }

  mqdaemon daemon(argv[0], APP_VERSION);

  std::cerr << "zmq-addr: " << ZMQ_ADDR << std::endl;
  std::cerr << "zmq-topic: " << ZMQ_TOPIC << std::endl;
  std::cerr << "cib-socket: " << cib_socket << std::endl;
  std::cerr << "cib-prefix: " << cib_prefix << std::endl;
  std::cerr << "cib-extension: " << cib_extension << std::endl;

  neat_writer writer(daemon, ZMQ_TOPIC, ZMQ_ADDR);
  writer.set_cib_socket(cib_socket);
  writer.set_cib_file(cib_prefix, cib_extension);

  daemon.run();

  return 0;
}
