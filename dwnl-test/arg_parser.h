#define PING_MODE_CONNECT (1)
#define PING_MODE_ECHO    (2)

struct app_config
{
  char *host;
  int port;
  char *path;
  //int mode;
  int count;
  int interval;
  int verbose;
};

void parse_args(int argc, char *argv[], struct app_config *cfg);
