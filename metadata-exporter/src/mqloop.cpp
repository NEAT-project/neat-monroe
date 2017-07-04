#include "mqloop.h"

mqloop::mqloop()
  : context(1)
{
}

mqloop::~mqloop()
{
}

zmq::context_t& mqloop::get_zmq_context()
{
  return context;
}

void mqloop::register_socket(zmq::socket_t *socket, std::function<bool()> handler)
{
  auto elem = sockets.find(socket);
  if (elem != sockets.end())
    throw std::runtime_error("Socket already registered");
  sockets[socket] = handler;
}

void mqloop::unregister_socket(zmq::socket_t *socket)
{
  auto elem = sockets.find(socket);
  if (elem == sockets.end())
    throw std::runtime_error("Socket not registered");
  sockets.erase(elem);
}
  
void mqloop::register_fd(int fd, std::function<bool()> handler)
{
  auto elem = fds.find(fd);
  if (elem != fds.end())
    throw std::runtime_error("File descriptor already registered");
  fds[fd] = handler;
}

void mqloop::unregister_fd(int fd)
{
  auto elem = fds.find(fd);
  if (elem == fds.end())
    throw std::runtime_error("File descriptor not registered");
  fds.erase(elem);
}

void mqloop::run()
{
  std::vector<zmq::pollitem_t> items;
  std::vector<std::function<bool()>> handlers;

  for(auto const &entry : sockets) {
    zmq::socket_t *socket = entry.first;
    std::function<bool()> handler = entry.second;
    zmq::pollitem_t item = { static_cast<void *>(*socket), 0, ZMQ_POLLIN, 0 };
    items.push_back(item);
    handlers.push_back(handler);
  }

  for(auto const &entry : fds) {
    int fd = entry.first;
    std::function<bool()> handler = entry.second;
    zmq::pollitem_t item = { NULL, fd, ZMQ_POLLIN, 0 };
    items.push_back(item);
    handlers.push_back(handler);
  }

  while(1)
  {
    zmq::poll(&items[0], items.size(), -1);
    int index = 0;
    for (auto it = items.begin(); it != items.end(); ++it, ++index) {
      if ((*it).revents & ZMQ_POLLIN) {
        bool result = handlers[index]();
        if (!result) {
          return;
        }
      }
    }
  }
}