
#define DWNL_HOST_LEN (255)
#define DWNL_PATH_LEN (255)
#define DWNL_BIND_IFNAME_LEN (32)

struct app_config
{
  char host[DWNL_HOST_LEN];
  int port;
  char path[DWNL_PATH_LEN];
  int count;
  int interval;
  int timeout;
  int verbose;
  char bind_ifname[DWNL_BIND_IFNAME_LEN];
};

void parse_args(int argc, char *argv[], struct app_config *cfg);
