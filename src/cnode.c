// Computational Node

#include <stdio.h>
#include <stdlib.h>

#include "net.h"
#include "proto.h"
#include "utils.h"

// static const struct node dnode = {.addr = "127.0.0.1", .port = 8000};

int main(int argc, char *argv[])
{
    int sockfd, ret;
    u_int16_t port;
    struct sockaddr_in client;

    if (argc != 2)
    {
        error("Usage: %s [Port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    if (port < 1 || port > 65535)
    {
        error("[Error]: Port number must be between 1 and 65535\n");
        exit(EXIT_FAILURE);
    }

    sockfd = init_server(port);
    sockfd = accept_conn(sockfd, &client);
    ret = reply(sockfd);
    if (ret < 0)
        info("[Info]: Oops!\n");

    exit(EXIT_SUCCESS);
}