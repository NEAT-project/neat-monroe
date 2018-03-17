#ifndef CELERWAY_LOGGER_H
#define CELERWAY_LOGGER_H 1

#define LOG_NONE    (0)
#define LOG_ERROR   (1)
#define LOG_WARNING (2)
#define LOG_INFO    (3)
#define LOG_DEBUG   (4)

void log_level(int level);

void log_error(const char* fmt, ...);
void log_warning(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_debug(const char* fmt, ...);

#endif //CELERWAY_LOGGER_H