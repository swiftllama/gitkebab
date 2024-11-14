/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
// NOTE: defines e.g. localtime_r on windows (assuming MingW)
#ifndef _POSIX_THREAD_SAFE_FUNCTIONS
#define _POSIX_THREAD_SAFE_FUNCTIONS
#endif
#include <time.h>

#define LOG_VERSION "0.1.0"

typedef struct {
  va_list ap;
  const char *fmt;
  const char *file;
  struct tm *time;
  void *udata;
  int line;
  int level;
  const char *component_label;
} log_Event;

typedef struct {
    int level;
    const char *label;
} log_Component;
    
typedef void (*log_LogFn)(log_Event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(component, ...) log_log(LOG_TRACE, __FILE__, __LINE__, &component, __VA_ARGS__)
#define log_debug(component, ...) log_log(LOG_DEBUG, __FILE__, __LINE__, &component, __VA_ARGS__)
#define log_info(component, ...)  log_log(LOG_INFO,  __FILE__, __LINE__, &component, __VA_ARGS__)
#define log_warn(component, ...)  log_log(LOG_WARN,  __FILE__, __LINE__, &component, __VA_ARGS__)
#define log_error(component, ...) log_log(LOG_ERROR, __FILE__, __LINE__, &component, __VA_ARGS__)
#define log_fatal(component, ...) log_log(LOG_FATAL, __FILE__, __LINE__, &component, __VA_ARGS__)
#define log_at_level(alevel, component, ...) log_log(alevel, __FILE__, __LINE__, &component, __VA_ARGS__)

const char* log_level_string(int level);
void log_set_lock(log_LockFn fn, void *udata);
void log_set_level(int level);
void log_set_quiet(bool enable);
int log_add_callback(log_LogFn fn, void *udata, int level);
int log_add_fp(FILE *fp, int level);

void log_log(int level, const char *file, int line, log_Component *component, const char *fmt, ...);

#endif
