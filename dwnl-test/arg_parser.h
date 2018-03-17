
struct app_config
{
  char *host;
  int port;
  char *path;
  int count;
  int interval;
  int timeout;
  int verbose;
  char *bind_ifname;
};

void parse_args(int argc, char *argv[], struct app_config *cfg);
