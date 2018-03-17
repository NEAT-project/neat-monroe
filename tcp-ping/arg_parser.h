#define PING_MODE_CONNECT (1)
#define PING_MODE_ECHO    (2)

#define PING_HOST_LEN (255)
#define PING_BIND_IFNAME_LEN (32)

struct app_config
{
  char host[PING_HOST_LEN];
  int port;
  int mode;
  int count;
  int interval;
  int timeout;
  int verbose;
  char bind_ifname[PING_BIND_IFNAME_LEN];
};

void parse_args(int argc, char *argv[], struct app_config *cfg);
