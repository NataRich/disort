// Distribution Node

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "net.h"
#include "proto.h"
#include "utils.h"

static const struct node cnodes[] = {
    {
        .addr = "127.0.0.1",
        .port = 7000,
    },
    {
        .addr = "127.0.0.1",
        .port = 7001,
    },
};

int main(int argc, char *argv[])
{
    int sockfd;
    struct packet meta;

    sockfd = init_client(cnodes[0].addr, cnodes[0].port);
    meta.seq = 0;
    meta.size = 10;
    meta.flags = LFM_F_FRST;
    if (confirm(sockfd, &meta, 2) < 0)
        info("[Info]: Oops!\n");

    exit(EXIT_SUCCESS);
}