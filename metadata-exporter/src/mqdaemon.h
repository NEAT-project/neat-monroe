#include "mqloop.h"

class mqdaemon : public mqloop
{
private:
  std::string name;
  std::string version;
  mqloop loop;
  int sfd;

  bool handle_signal();

public:
  mqdaemon(const std::string& name, const std::string& version);
  ~mqdaemon();
};
