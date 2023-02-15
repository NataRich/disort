// Distribution Node

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>

#include "net.h"
#include "files.h"
#include "utils.h"

#define IMPROP -1
#define PACKERR -2
#define SOCKERR -3
#define FILEERR -4
#define UNKNOWN -10

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

void try_ftransfer(int sockfd, pid_t tid, char *path, int *keep, int *err);
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
    free(tids);
    exit(EXIT_SUCCESS);
}

void *transfer_thread(void *args)
{
    pid_t self;
    int sockfd, keep, err;
    struct thread_args *targs;

    err = 0;
    keep = 1;
    self = syscall(__NR_gettid);
    targs = (struct thread_args *)args;

    do
    {
        sockfd = init_client(targs->addr, targs->port);
        try_ftransfer(sockfd, self, targs->path, &keep, &err);
    } while (keep == 1);

    if (err != 0)
        error("[Error][t%d]: Error code %d\n", self, err);
    else
        info("[Info][t%d]: Successful ftransfer()\n", self);
    free(args);
    return NULL;
}

void try_ftransfer(int sockfd, pid_t tid, char *path, int *keep, int *err)
{
    int ret, rcvd;
    struct packet *rcv_pkt;

    do
    {
        ret = ftransfer(sockfd, path);
        if (ret == SUCCESS)
        {
            rcvd = lfm_recv(sockfd, &rcv_pkt); // should receive a reply or retry packet
            if (rcvd == ERR_SOCKIO)
            {
                error("[Error][t%d]: Improper termination of socket (sockerr)\n", tid);
                *err = IMPROP;
                *keep = 0; // thread exits
                close(sockfd);
                return;
            }
            else if (IS_RETRY_SET(rcv_pkt->flags) == 1)
                continue;
            else if (IS_REPLY_SET(rcv_pkt->flags) == 1)
            {
                info("[Info][t%d]: Socket termination\n", tid);
                *keep = 0; // thread_exits
                close(sockfd);
                free(rcv_pkt);
                return;
            }
            else
            {
                error("[Error][t%d]: Improper termination of socket (packeterr)\n", tid);
                *err = PACKERR;
                *keep = 0; // thread exits
                close(sockfd);
                free(rcv_pkt);
                return;
            }
        }
        else if (ret == ERR_SOCKIO)
        {
            error("[Error][t%d]: Socket error. Initiating a new socket conection\n", tid);
            *err = SOCKERR;
            close(sockfd);
            return;
        }
        else if (ret == ERR_CONFIRM) // may end up in ERR_SOCKIO
        {
            error("[Error][t%d]: Confirmation failure. Retrying\n", tid);
            continue;
        }
        else if (ret == ERR_FILEIO)
        {
            error("[Error][t%d]: File I/O failure\n", tid);
            *err = FILEERR;
            *keep = 0; // thread exits due to error
            close(sockfd);
            return;
        }
        else
        {
            error("[Error][t%d]: Unknown error type\n", tid);
            *err = UNKNOWN;
            *keep = 0; // thread exits due to error
            close(sockfd);
            return;
        }
    } while (1);
}