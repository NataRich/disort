// Computational Node

#include <stdio.h>
#include <stdlib.h>

#include "net.h"
#include "files.h"
#include "utils.h"

#define MAX_RETRY 5

// static const struct node dnode = {.addr = "127.0.0.1", .port = 8000};

int main(int argc, char *argv[])
{
    int sockfd, ret, flag;
    u_int16_t port;
    struct packet pkt;
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

    do
    {
        flag = 0;
        ret = freceive(sockfd, argv[2]);
        switch (ret)
        {
        case SUCCESS:
            pkt.seq = 0;
            pkt.size = 0;
            pkt.flags = LFM_F_REPLY;
            ret = lfm_send(sockfd, &pkt);
            if (ret < 0)
            {
                error("[Error]: Failed to send retry request\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            break;

        case ERR_FILEIO:
        case ERR_SOCKIO:
            error("[Error]: File/Socket I/O failure\n");
            close(sockfd);
            exit(EXIT_FAILURE);

        case ERR_PARTIAL:
            flag = 1;
            pkt.seq = 0;
            pkt.size = 0;
            pkt.flags = LFM_F_RETRY;
            ret = lfm_send(sockfd, &pkt);
            if (ret < 0)
            {
                error("[Error]: Failed to send retry request\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            break;

        default:
            error("[Error]: Unknown error type\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    } while (flag > 0);

    close(sockfd);
    exit(EXIT_SUCCESS);
}