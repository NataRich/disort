// Computational Node

#include <stdio.h>
#include <stdlib.h>

#include "net.h"
#include "files.h"
#include "utils.h"

// static const struct node dnode = {.addr = "127.0.0.1", .port = 8000};

int main(int argc, char *argv[])
{
    int sockfd;
    u_int16_t port;
    struct sockaddr_in client;

    if (argc != 3)
    {
        error("Usage: %s [Port] [Path]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    if (port < 1 || port > 65535)
    {
        error("[Error]: Port number must be between 1 and 65535\n");
        error("Usage: %s [Port] [Path]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (fexists(argv[2]) == 0)
    {
        error("[Error]: File (%s) already exists\n", argv[2]);
        error("Usage: %s [Port] [Path]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sockfd = init_server(port);
    sockfd = accept_conn(sockfd, &client);
    freceive(sockfd, argv[2]);
    close(sockfd);

    exit(EXIT_SUCCESS);
}