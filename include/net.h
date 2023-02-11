#ifndef _NET_H_
#define _NET_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "utils.h"

#define kQueueMax 5           // listen queue size
#define kDataBufferMax 655000 // data buffer size

struct node
{
    char addr[30]; // IPv4 address
    unsigned short port;
};

/**
 * Initializes a socket connection to the given server.
 *
 * @param ip The IPv4 address of the server.
 * @param port The associated port of the process.
 * @return The socket handle.
 */
int init_client(const char *ip, u_short port);

/**
 * Initializes a socket listening to the given port.
 *
 * @param port The port assigned to the current process.
 * @return The socket handle.
 */
int init_server(u_short port);

/**
 * Accepts a client connection when the socket is ready.
 *
 * @param handle The listening socket.
 * @param client The place where client info will be stored.
 * @return The new socket handle for communication with the client.
 */
int accept_conn(int handle, struct sockaddr_in *client);

/**
 * Sends the given data to the socket.
 *
 * @param handle The established socket.
 * @param data The data to be sent.
 * @param size The size of data (max: 1MB).
 * @param more The flag telling whether there are more data.
 *
 * @note Don't set more field to true for now!
 */
void send_data(int handle, void *data, size_t size, bool more);

/**
 * Receives data from the socket.
 *
 * @param handle The established socket.
 * @param buffer The buffer to hold received data.
 * @param size The size of the buffer, and of the received data.
 */
void recv_data(int handle, void *buffer, size_t size);

// TODO: consider multithreading in socket

// TODO: disk I/O

// TODO: multithreaded external sorting

#endif