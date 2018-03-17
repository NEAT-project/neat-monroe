#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <time.h>
#include <errno.h>

#include "version.h"
#include "arg_parser.h"
#include "logger.h"

#define LOCAL_ADDR_LEN (32)

struct flow_info
{
  struct timespec ts1;
  struct timespec tsinit;
  struct timespec ts2;
  struct app_config cfg;
  int iter;
  struct timespec ts_app_start;
  int len;
  char local_addr[LOCAL_ADDR_LEN];
  uint16_t local_port;
};

int check_app_timeout(struct flow_info *fi)
{
  struct timespec ts;

  if (fi->cfg.timeout > 0) {
    clock_gettime(CLOCK_REALTIME, &ts);
    if (ts.tv_sec - fi->ts_app_start.tv_sec > fi->cfg.timeout) {
      log_warning("App timeout reached");
      return 1;
    }
  }

  return 0;
}

int dwnl_test_run(struct flow_info *fi)
{
  int err = 0;
  int sockfd = -1;
  struct hostent *he = NULL;
  struct sockaddr_in serv_addr, local_addr;
  int addr_len = sizeof(struct sockaddr);
  char send_buffer[256];
  char recv_buffer[2048];
  int len = 0;

  clock_gettime(CLOCK_REALTIME, &fi->ts1);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    err = -1;
    log_error("Failed to create a socket");
    goto cleanup;
  }

  if (fi->cfg.bind_ifname) {
    err = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,
      (void *)fi->cfg.bind_ifname, strlen(fi->cfg.bind_ifname));
    if (err) {
      log_error("Binding to %s failed. %s",
        fi->cfg.bind_ifname, strerror(errno));
      goto cleanup;
    }
  }

  he = gethostbyname(fi->cfg.host);
  if (!he) {
    err = -1;
    log_error("Failed to resolve host name %s", fi->cfg.host);
    goto cleanup;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(fi->cfg.port);
  serv_addr.sin_addr = *((struct in_addr **)he->h_addr_list)[0];

  err = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (err) {
    log_error("Failed to connect to %s", fi->cfg.host);
    goto cleanup;
  }

  err = getsockname(sockfd, (struct sockaddr *)&local_addr, (socklen_t *)&addr_len);
  if (err) {
    log_error("Failed to obtain local socket address. %s", strerror(errno));
    goto cleanup;
  }
  snprintf(fi->local_addr, LOCAL_ADDR_LEN, "%s", inet_ntoa(local_addr.sin_addr));
  fi->local_port = ntohs(local_addr.sin_port);

  clock_gettime(CLOCK_REALTIME, &fi->tsinit);

  snprintf(send_buffer, sizeof(send_buffer),
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "User-agent: dwnl-test\r\n"
    "Connection: close\r\n"
    "\r\n",
    fi->cfg.path, fi->cfg.host);

  len = send(sockfd, send_buffer, sizeof(send_buffer), 0);
  if (len != sizeof(send_buffer)) {
    err = -1;
    log_error("send failed");
    goto cleanup;
  }

  fi->len = 0;
  while(1) {
    len = recv(sockfd, recv_buffer, sizeof(recv_buffer), 0);
    if (len < 0) {
      err = -1;
      log_error("recv failed");
      goto cleanup;
    } else if (len == 0) {
      log_debug("recv EOF");
      break;
    } else {
      log_debug("recv received %d bytes", len);
      fi->len += len;
      if (check_app_timeout(fi)) {
        break;
      }
    }
  }

  clock_gettime(CLOCK_REALTIME, &fi->ts2);

cleanup:
  if (sockfd >= 0) {
    close(sockfd);
  }

  return err;
}

int main(int argc, char *argv[])
{
  int err = 0;
  struct flow_info fi;
  memset(&fi, 0, sizeof(struct flow_info));

  parse_args(argc, argv, &fi.cfg);

  log_level(fi.cfg.verbose);
  log_info("%s v%s started", APP_NAME, APP_VERSION);
  log_info("Host %s", fi.cfg.host);
  log_info("Port %d", fi.cfg.port);
  log_info("Path %s", fi.cfg.path);
  log_info("Count %d", fi.cfg.count);
  log_info("Interval %d", fi.cfg.interval);
  log_info("Timeout %d", fi.cfg.timeout);
  log_info("Bind %s", fi.cfg.bind_ifname);
  log_info("Verbose %d", fi.cfg.verbose);

  clock_gettime(CLOCK_REALTIME, &fi.ts_app_start);

  fi.iter = 0;
  while(fi.iter < fi.cfg.count) {
    fi.iter += 1;
    err = dwnl_test_run(&fi);
    if (err) {
      log_error("dwnl_test_run failed");
      goto cleanup;
    }

    if (fi.tsinit.tv_nsec < fi.ts1.tv_nsec) {
      fi.tsinit.tv_nsec += 1000000000;
      fi.tsinit.tv_sec--;
    }
    if (fi.ts2.tv_nsec < fi.ts1.tv_nsec) {
      fi.ts2.tv_nsec += 1000000000;
      fi.ts2.tv_sec--;
    }
    fprintf(stdout, "dwnl-test\t%d\t%s\t%s\t%d\t%s\t%d\t%ld.%09ld\t%ld.%09ld\n",
      fi.iter, fi.local_addr, fi.cfg.host, fi.cfg.port, fi.cfg.path, fi.len,
      (long)(fi.tsinit.tv_sec - fi.ts1.tv_sec),
      fi.tsinit.tv_nsec - fi.ts1.tv_nsec,
      (long)(fi.ts2.tv_sec - fi.ts1.tv_sec),
      fi.ts2.tv_nsec - fi.ts1.tv_nsec);

    if (check_app_timeout(&fi)) {
        break;
    }

    sleep(fi.cfg.interval);
  }

cleanup:
  if (fi.cfg.host) {
    free(fi.cfg.host);
  }

  if (fi.cfg.path) {
    free(fi.cfg.path);
  }

   if (fi.cfg.bind_ifname) {
    free(fi.cfg.bind_ifname);
  }

  log_info("%s v%s terminated", APP_NAME, APP_VERSION);
  return err;
}
