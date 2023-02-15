// Computational Node

#include <stdio.h>
#include <stdlib.h>

#include "net.h"
#include "sort.h"
#include "files.h"
#include "utils.h"

#define USABLEMEM 1024 * 1024 * 1024 // 1GB

// static const struct node dnode = {.addr = "127.0.0.1", .port = 8000};

void try_freceive(int sockfd, char *path, int *keep);

int main(int argc, char *argv[])
{
    int o_sockfd, n_sockfd, keep;
    u_int16_t port;
    struct sockaddr_in client;
    char path[30];

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

    o_sockfd = init_server(port);

    keep = 1;
    do
    {
        n_sockfd = accept_conn(o_sockfd, &client);
        try_freceive(n_sockfd, argv[2], &keep);
    } while (keep == 1);

    memset(path, 0, 30);
    memcpy(path, argv[2], strlen(argv[2]) + 1);
    ext_sort(path, USABLEMEM);

    info("[Info]: Successful freceive()\n");
    exit(EXIT_SUCCESS);
}

void try_freceive(int sockfd, char *path, int *keep)
{
    int ret;
    struct packet pkt;

    do
    {
        ret = freceive(sockfd, path);
        if (ret == SUCCESS)
        {
            pkt.seq = pkt.size = 0;
            pkt.flags = LFM_F_REPLY;
            ret = lfm_send(sockfd, &pkt);
            if (ret <= 0)
                error("[Error]: Failed to send reply request\n");

            *keep = 0; // program exits
            close(sockfd);
            return;
        }
        else if (ret == ERR_PARTIAL)
        {
            pkt.seq = pkt.size = 0;
            pkt.flags = LFM_F_RETRY;
            ret = lfm_send(sockfd, &pkt);
            if (ret <= 0)
            {
                error("[Error]: Failed to send retry request. Waiting for new socket connection\n");
                close(sockfd);
                return;
            }

            break; // wait for new confirmation
        }
        else if (ret == ERR_CONFIRM)
        {
            error("[Error]: Confirmation failure. Waiting for new socket connection\n");
            close(sockfd);
            return;
        }
        else if (ret == ERR_SOCKIO)
        {
            error("[Error]: Socket error. Waiting for new socket connection\n");
            close(sockfd);
            return;
        }
        else if (ret == ERR_FILEIO)
        {
            error("[Error]: File I/O failure\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        else
        {
            error("[Error]: Unknown error type\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    } while (1);
}