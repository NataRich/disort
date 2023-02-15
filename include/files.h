#ifndef _FILES_H_
#define _FILES_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "proto.h"
#include "utils.h"

/**
 * Checks if the given file exists.
 *
 * @param 0 on existence.
 */
int fexists(char *path);

/**
 * Gets the file size in bytes.
 *
 * @param path The file path.
 * @return negative on error and positive means file size in bytes.
 */
long fgetsize(const char *path);

/**
 * Creates a file if not exist, or erases the data otherwise.
 *
 * @param path The path of the new file.
 * @return 0 on success and -1 on error.
 */
int fcreate(char *path);

/**
 * Transfers the entire file via socket.
 *
 * @param sockfd The socket file descriptor.
 * @param path The path to an existing file.
 * @return 0 on success and -1 on error.
 */
int ftransfer(int sockfd, char *path);

/**
 * Receives an entire file via socket.
 *
 * @param sockfd The socket file descriptor.
 * @param path The path to which data are written.
 * @return 0 on success and negative on errors.
 */
int freceive(int sockfd, char *path);

#endif