// Distribution Node

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>

#include "net.h"
#include "files.h"
#include "utils.h"

#define MAX_RETRY 5

struct thread_args
{
    char addr[30];
    char path[30];
    u_int16_t port;
};

static const struct node cnodes[] = {
    {
        .addr = "127.0.0.1",
        .port = 7000,
    },
};

void *transfer_thread(void *args);

int main(int argc, char *argv[])
{
    int i, ncnt;
    pthread_t *tids;

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

    tids = (pthread_t *)malloc(ncnt * sizeof(pthread_t));
    if (tids == NULL)
    {
        error("[Error]: malloc() failed\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < ncnt; i++)
    {
        struct thread_args *args;

        args = (struct thread_args *)malloc(sizeof(struct thread_args));
        if (args == NULL)
        {
            error("[Error]: malloc() failed\n");
            exit(EXIT_FAILURE);
        }
        memcpy(args->addr, cnodes[i].addr, 30);
        memcpy(args->path, argv[i + 1], strlen(argv[i + 1]) + 1);
        args->port = cnodes[i].port;
        pthread_create(&tids[i], NULL, transfer_thread, args);
    }

    for (i = 0; i < ncnt; i++)
        pthread_join(tids[i], NULL);

    info("[Info]: Executed all\n");

    exit(EXIT_SUCCESS);
}

void *transfer_thread(void *args)
{
    pid_t self;
    int sockfd, ret, flag, nretry;
    struct packet *pkt;
    struct thread_args *targs;

    pkt = NULL;
    self = syscall(__NR_gettid);
    targs = (struct thread_args *)args;
    sockfd = init_client(targs->addr, targs->port);
    do
    {
        flag = 0;
        nretry = 0;
        ret = ftransfer(sockfd, targs->path);
        switch (ret)
        {
        case SUCCESS:
            break;

        case ERR_FILEIO:
            error("[Error][t%d]: File I/O failure\n", self);
            close(sockfd);
            free(args);
            return NULL;
        case ERR_CONFIRM:
            error("[Error][t%d]: Confirmation failure\n", self);
            close(sockfd);
            free(args);
            return NULL;

        default:
            error("[Error][t%d]: Unknown error type\n", self);
            close(sockfd);
            free(args);
            return NULL;
        }

        do
        {
            if (pkt != NULL)
                free(pkt);

            ret = lfm_recv(sockfd, &pkt);
            if (ret < 0 && nretry < MAX_RETRY)
            {
                nretry++;
                info("[Info][t%d]: Waiting for reply...\n", self);
                sleep(5); // sleep 5 seconds because lfm_recv default timeout is 15 seconds
            }
            else if (ret == 0)
            {
                error("[Error][t%d]: Socket closed without reply\n", self);
                close(sockfd);
                free(args);
                return NULL;
            }
            else if (ret < MAX_SOCK_BUFFER)
            {
                error("[Error][t%d]: Incomplete packet received\n", self);
                close(sockfd);
                free(args);
                return NULL;
            }
            else if (IS_REPLY_SET(pkt->flags) == 1)
            {
                info("[Info][t%d]: Successful transfer\n", self);
                break;
            }
            else if (IS_RETRY_SET(pkt->flags) == 1)
            {
                info("[Info][t%d]: Retransfering\n", self);
                flag = 1;
                break;
            }
            else
            {
                error("[Error][t%d]: Incorrect communication\n", self);
                close(sockfd);
                free(args);
                return NULL;
            }
        } while (nretry < MAX_RETRY);

        if (pkt != NULL)
            free(pkt);

    } while (flag > 0);

    sleep(5); // next step...

    info("[Info][t%d]: TO THE END!\n", self);
    close(sockfd);
    free(args);
    return NULL;
}