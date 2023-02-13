#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024

/**
 * Checks if the current process is a child process.
 *
 * @return 0 if the current process is a child else 1.
 */
int is_child();

/**
 * Exits the process safely considering child/parent processes.
 */
void safe_exit(int status);

/**
 * Prints formatted string to standard error.
 *
 * @param format The format string.
 */
void error(const char *format, ...);

/**
 * Prints formatted string to standard output.
 *
 * @param format The format string.
 */
void info(const char *format, ...);

#endif