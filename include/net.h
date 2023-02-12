#ifndef _NET_H_
#define _NET_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "utils.h"

#define MAX_SOCK_QUEUE 5      // listen queue size
#define MAX_SOCK_BUFFER 65536 // socket data buffer size

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
 * @return The socket file descriptor.
 */
int init_client(const char *ip, u_short port);

/**
 * Initializes a socket listening to the given port.
 *
 * @param port The port assigned to the current process.
 * @return The socket file descriptor.
 */
int init_server(u_short port);

/**
 * Accepts a client connection when the socket is ready.
 *
 * @param sockfd The listening socket file descriptor.
 * @param client The place where client info will be stored.
 * @return The new socket file descriptor for communication with the client.
 */
int accept_conn(int sockfd, struct sockaddr_in *client);

/**
 * Sends the given data to the socket.
 *
 * @param sockfd The established socket file descriptor.
 * @param data The data to be sent.
 * @param size The size of data (max: 64KB).
 * @param tmot The timeout for send operation.
 */
void send_data(int sockfd, void *data, size_t size, struct timeval *tmot);

/**
 * Receives data from the socket.
 *
 * @param sockfd The established socket file descriptor.
 * @param buffer The buffer to hold received data.
 * @param size The size of the buffer, and of the received data.
 * @param tmot The timeout for receive operation.
 */
void recv_data(int sockfd, void *buffer, size_t size, struct timeval *tmot);

#endif