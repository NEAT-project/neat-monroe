#include <stdio.h>
#include <stdarg.h>
#include <time.h> 

#include "logger.h"

static int app_level = LOG_ERROR;

static const char *app_level_str[] = {
  "NON",
  "ERR",
  "WRN",
  "INF",
  "DEB"
};


inline int min(int a, int b) { return a < b ? a : b; }
inline int max(int a, int b) { return a > b ? a : b; }

void log_level(int level)
{
    app_level = min(max(level, LOG_NONE),LOG_DEBUG);
}

static void log(int level, const char* fmt, va_list args)
{
  struct timespec ts;
  char msg_buf[1024];
  char ts_buf[20];
  int len = 0;

  if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
    fputs("Internal logger error (clock_gettime)\n", stderr);
    return;
  }

  len = strftime(ts_buf, sizeof(ts_buf), "%F %T", localtime(&ts.tv_sec));
  if (len < 0) {
    fputs("Internal logger error (strftime)\n", stderr);
    return;
  }

  len = vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
  if (len < 0) {
    fputs("Internal logger error (vsnprintf)\n", stderr);
    return;
  }

  fprintf(stderr, "[%s.%03ld %s] %s\n",
    ts_buf,
    ts.tv_nsec / 1000000,
    app_level_str[level],
    msg_buf);
}

void log_error(const char* fmt, ...)
{
  if (app_level >= LOG_ERROR) {
    va_list args;
    va_start(args, fmt);
    log(LOG_ERROR, fmt, args);
    va_end (args);

  }
}

void log_warning(const char* fmt, ...)
{
  if (app_level >= LOG_WARNING) {
    va_list args;
    va_start(args, fmt);
    log(LOG_WARNING, fmt, args);
    va_end (args);

  }
}

void log_info(const char* fmt, ...)
{
  if (app_level >= LOG_INFO) {
    va_list args;
    va_start(args, fmt);
    log(LOG_INFO, fmt, args);
    va_end (args);

  }
}

void log_debug(const char* fmt, ...)
{
  if (app_level >= LOG_DEBUG) {
    va_list args;
    va_start(args, fmt);
    log(LOG_DEBUG, fmt, args);
    va_end (args);

  }
}
