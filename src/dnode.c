// Distribution Node

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "net.h"
#include "files.h"
#include "utils.h"

static const struct node cnodes[] = {
    {
        .addr = "127.0.0.1",
        .port = 7000,
    },
};

int main(int argc, char *argv[])
{
    int i, sockfd, ncnt;
    u_int16_t client_port;
    char client_ip[30];

    ncnt = sizeof(cnodes) / sizeof(struct node);
    if (argc != ncnt + 1)
    {
        error("[Error]: Incorrect number of data files and nodes\n");
        error("Usage: %s [PATHS...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < ncnt; i++)
    {
        if (fexists(argv[i + 1]) != 0)
        {
            error("[Error]: File (%s) does not exist\n", argv[i + 1]);
            error("Usage: %s [PATHS...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < ncnt; i++)
    {
        memcpy(client_ip, cnodes[i].addr, 30);
        client_port = cnodes[i].port;
        sockfd = init_client(client_ip, client_port);
        if (ftransfer(sockfd, argv[i + 1]) < 0)
        {
            info("[Info]: Failed to transfer file (%s) to mode (%s:%d)\n",
                 argv[i + 1], client_ip, client_port);
        }
        close(sockfd);
    }

    exit(EXIT_SUCCESS);
}