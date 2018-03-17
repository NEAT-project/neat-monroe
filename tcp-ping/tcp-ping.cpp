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
  struct timespec ts2;
  struct app_config cfg;
  int iter;
  struct timespec ts_app_start;
  char local_addr[LOCAL_ADDR_LEN];
  uint16_t local_port;
};

void print_rtt(struct flow_info *fi, int mode)
{
  clock_gettime(CLOCK_REALTIME, &fi->ts2);

  if (fi->ts2.tv_nsec < fi->ts1.tv_nsec) {
    fi->ts2.tv_nsec += 1000000000;
    fi->ts2.tv_sec--;
  }

  fi->iter += 1;

  fprintf(stdout, "tcp-ping\t%d\t%s\t%s\t%d\t%s\t%ld.%09ld\n",
    fi->iter, fi->local_addr, fi->cfg.host, fi->cfg.port,
    mode == PING_MODE_ECHO ? "ECHO" : "CONN",
    (long)(fi->ts2.tv_sec - fi->ts1.tv_sec),
    fi->ts2.tv_nsec - fi->ts1.tv_nsec);
}

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

int tcp_ping_connect(struct flow_info *fi)
{
  int err = 0;
  int sockfd = -1;
  struct hostent *he = NULL;
  struct sockaddr_in serv_addr, local_addr;
  int len = sizeof(struct sockaddr);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    err = -1;
    log_error("Failed to create a socket. %s", strerror(errno));
    goto cleanup;
  }

  if (strlen(fi->cfg.bind_ifname) > 0) {
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

  err = getsockname(sockfd, (struct sockaddr *)&local_addr, (socklen_t *)&len);
  if (err) {
    log_error("Failed to obtain local socket address. %s", strerror(errno));
    goto cleanup;
  }
  snprintf(fi->local_addr, LOCAL_ADDR_LEN, "%s", inet_ntoa(local_addr.sin_addr));
  fi->local_port = ntohs(local_addr.sin_port);

cleanup:
  if (err) {
    if (sockfd >= 0) {
      close(sockfd);
    }
    return -1;
  }

  return sockfd;
}

int tcp_ping_run_connect(struct flow_info *fi)
{
  int err = 0;
  int sockfd = -1;

  fi->iter = 0;

  while(fi->iter < fi->cfg.count) {
    if (fi->iter > 0) {
      sleep(fi->cfg.interval);
    }

    clock_gettime(CLOCK_REALTIME, &fi->ts1);

    sockfd = tcp_ping_connect(fi);
    if (sockfd < 0) {
      err = -1;
      log_error("tcp_ping_connect failed");
      goto cleanup;
    }

    print_rtt(fi, PING_MODE_CONNECT);

    close(sockfd);

    if (check_app_timeout(fi)) {
      break;
    }
  }

cleanup:

  return err;
}

int tcp_ping_run_echo(struct flow_info *fi)
{
  int err = 0;
  int sockfd = -1;
  struct hostent *he = NULL;
  struct sockaddr_in serv_addr;
  int len = 0;
  char send_buffer[] = "12345678901234567890123456789012345678901234567890123456789012\n";
  char recv_buffer[64];

  fi->iter = 0;
  clock_gettime(CLOCK_REALTIME, &fi->ts1);

  sockfd = tcp_ping_connect(fi);
  if (sockfd < 0) {
    err = -1;
    log_error("tcp_ping_connect failed");
    goto cleanup;
  }

  print_rtt(fi, PING_MODE_CONNECT);

  while(fi->iter < fi->cfg.count) {
    // Wait interval seconds before next ping
    // Note that first ping has already completed - connect ping
    sleep(fi->cfg.interval);

    clock_gettime(CLOCK_REALTIME, &fi->ts1);

    // Send ping data to the echo server
    len = send(sockfd, send_buffer, sizeof(send_buffer), 0);
    if (len != sizeof(send_buffer))
    {
      err = -1;
      log_error("send failed");
      goto cleanup;
    }

    // Receive the echo reply from the server
    len = recv(sockfd, recv_buffer, sizeof(recv_buffer), 0);
    if (len != sizeof(recv_buffer)) {
      err = -1;
      log_error("recv failed");
      goto cleanup;
    }

    print_rtt(fi, PING_MODE_ECHO);

    if (check_app_timeout(fi)) {
      break;
    }
  }

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
  log_info("Mode %s", fi.cfg.mode == PING_MODE_ECHO ? "ECHO" : "CONN");
  log_info("Count %d", fi.cfg.count);
  log_info("Interval %d", fi.cfg.interval);
  log_info("Timeout %d", fi.cfg.timeout);
  log_info("Bind %s", fi.cfg.bind_ifname);
  log_info("Verbose %d", fi.cfg.verbose);

  clock_gettime(CLOCK_REALTIME, &fi.ts_app_start);
  
  if (fi.cfg.mode == PING_MODE_CONNECT) {
    log_debug("PING_MODE_CONNECT");
    err = tcp_ping_run_connect(&fi);
  }
  else {
    log_debug("PING_MODE_ECHO");
    err = tcp_ping_run_echo(&fi);
  }

  log_info("%s v%s terminated", APP_NAME, APP_VERSION);
  return err;
}
