#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#define kBufferSize 1024

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