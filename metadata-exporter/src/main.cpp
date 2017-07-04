#include <zmq.hpp>
#include <json-c/json.h>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/wait.h>

#include "mqdaemon.h"
#include "neat_writer.h"

#define NEAT_METADATA_EXPORTER_VERSION "1.0.0"

int main(int argc, char **argv)
{
  mqdaemon daemon("neat-metadata-exporter", NEAT_METADATA_EXPORTER_VERSION);
  
  neat_writer writer(daemon);
  writer.set_cib_socket("/monroe/cib/neat_cib_socket");
  writer.set_cib_file("/monroe/cib/", ".cib");

  daemon.run();

  return 0;
}
