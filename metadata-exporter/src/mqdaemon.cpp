#include <iostream>

#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/wait.h>

#include "mqdaemon.h"

mqdaemon::mqdaemon(const std::string& name, const std::string& version)
  : name(name), version(version)
{
  std::cerr << name << " daemon " << version << " started" << std::endl;

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);
  if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
    std::cerr << "sigprocmask failed" << std::endl;
  }
  sfd = signalfd(-1, &mask, 0);
  if (sfd == -1) {
    std::cerr << "signalfd failed" << std::endl;
  }

  register_fd(sfd, std::bind(&mqdaemon::handle_signal, this));
}
  
mqdaemon::~mqdaemon()
{
  unregister_fd(sfd);
  std::cerr << name << " daemon " << version << " stopped" << std::endl;
}

bool mqdaemon::handle_signal()
{
  bool result = true;
  struct signalfd_siginfo fdsi;
  //TODO Check size returned by read
  /*ssize_t s =*/ read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
  if (fdsi.ssi_signo == SIGINT) {
    std::cerr << "Got SIGINT" << std::endl;
    result = false;
  } else if (fdsi.ssi_signo == SIGTERM) {
    std::cerr << "Got SIGTERM" << std::endl;
    result = false;
  } else {
    std::cerr << "Read unexpected signal" << std::endl;
  }
  return result;
}