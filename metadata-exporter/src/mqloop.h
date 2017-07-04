#ifndef CELERWAY_MQLOOP
#define CELERWAY_MQLOOP 1

#include <map>
#include <functional>
#include <zmq.hpp>

class mqloop
{
private:
  std::map<zmq::socket_t *, std::function<bool()> > sockets;
  std::map<int, std::function<bool()> > fds;
  zmq::context_t context;

public:
  mqloop();
  ~mqloop();

  zmq::context_t& get_zmq_context();

  void register_socket(zmq::socket_t *socket, std::function<bool()> handler);
  void unregister_socket(zmq::socket_t *socket);
  
  void register_fd(int fd, std::function<bool()> handler);
  void unregister_fd(int fd);

  //TODO register_timer
  //TODO unregister_timer

  // Runs until one of the registered sockets/timers hanlers returns false
  void run();
};

#endif // CELERWAY_MQLOOP