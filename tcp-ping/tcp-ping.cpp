#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <time.h>

#include "version.h"
#include "arg_parser.h"

struct flow_info
{
  struct timespec ts1;
  struct timespec ts2;
  struct app_config cfg;
  int iter;
};

int tcp_ping_run(struct flow_info *fi)
{
  int err = 0;
  int sockfd = -1;
  struct hostent *he = NULL;
  struct sockaddr_in serv_addr;
  int len = 0;
  char send_buffer[] = "12345678901234567890123456789012345678901234567890123456789012\n";
  char recv_buffer[64];
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "ERROR: failed to create a socket\n");
    goto cleanup;
  }

  he = gethostbyname(fi->cfg.host);
  if (!he) {
    fprintf(stderr, "ERROR: failed to resolve host name %s\n", fi->cfg.host);
    goto cleanup;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(fi->cfg.port); 
  serv_addr.sin_addr = *((struct in_addr **)he->h_addr_list)[0];

  clock_gettime(CLOCK_REALTIME, &fi->ts1);
  
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
    fprintf(stderr, "ERROR: failed to connect to %s\n", fi->cfg.host);
    goto cleanup;
  }

  if (fi->cfg.mode == PING_MODE_CONNECT) {
    clock_gettime(CLOCK_REALTIME, &fi->ts2);
    if (fi->ts2.tv_nsec < fi->ts1.tv_nsec) {
      fi->ts2.tv_nsec += 1000000000;
      fi->ts2.tv_sec--;
    }
    fi->iter += 1;
    fprintf(stdout, "%d %ld.%09ld\n", fi->iter, 
      (long)(fi->ts2.tv_sec - fi->ts1.tv_sec),
      fi->ts2.tv_nsec - fi->ts1.tv_nsec);

    close(sockfd);
    goto cleanup;
  } else {
    while(fi->iter < fi->cfg.count) {
     
      clock_gettime(CLOCK_REALTIME, &fi->ts1);

      // Send ping data to the echo server
      len = send(sockfd, send_buffer, sizeof(send_buffer), 0);
      if (len != sizeof(send_buffer))
      {
        fprintf(stderr, "ERROR: send failed\n");
        goto cleanup;
      }
         
      // Receive the echo reply from the server
      len = recv(sockfd, recv_buffer, sizeof(recv_buffer), 0);
      if (len != sizeof(recv_buffer)) {
        fprintf(stderr, "ERROR: recv failed\n");
        goto cleanup;
      }

      // Print ping's RTT to stdout
      clock_gettime(CLOCK_REALTIME, &fi->ts2);
      if (fi->ts2.tv_nsec < fi->ts1.tv_nsec) {
        fi->ts2.tv_nsec += 1000000000;
        fi->ts2.tv_sec--;
      }
      fi->iter += 1;
      fprintf(stdout, "%d %ld.%09ld\n", fi->iter, 
        (long)(fi->ts2.tv_sec - fi->ts1.tv_sec),
        fi->ts2.tv_nsec - fi->ts1.tv_nsec);

      // Wait interval seconds before next ping
      sleep(fi->cfg.interval);
    }
  }

cleanup:
  return err;
}

int main(int argc, char *argv[])
{
  int err = 0;
  struct flow_info fi;
  fprintf(stderr, "INFO: %s started\n", APP_NAME);

  fi.iter = 0;

  parse_args(argc, argv, &fi.cfg);

  if (fi.cfg.mode == PING_MODE_CONNECT) {
    fprintf(stderr, "INFO: PING_MODE_CONNECT\n");
    while(fi.iter < fi.cfg.count) {
      err = tcp_ping_run(&fi);
      if (err) {
        fprintf(stderr, "ERROR: %s - tcp_ping_run failed\n", __FUNCTION__);
        goto cleanup;
      }
      sleep(fi.cfg.interval);
    }
  }
  else {
    fprintf(stderr, "INFO: PING_MODE_ECHO\n");
    err = tcp_ping_run(&fi);
    if (err) {
      fprintf(stderr, "ERROR: %s - tcp_ping_run failed\n", __FUNCTION__);
      goto cleanup;
    }
  }

cleanup:
  if (fi.cfg.host) {
    free(fi.cfg.host);
  }

  fprintf(stderr, "INFO: %s terminated\n", APP_NAME);
  return err;
}
