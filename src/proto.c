#include "proto.h"

static struct timeval glob_tv = {
    .tv_sec = 15,
    .tv_usec = 0,
};

int confirm(int sockfd, struct packet *pkt, short retry)
{
    int ret;
    struct packet *rcv_pkt;
    char sock_buf[MAX_SOCKET_BUF];

    if (pkt == NULL)
    {
        info("[Info]: File meta packet must not be null\n");
        return -1;
    }

    rcv_pkt = NULL;
    retry = retry < 0 ? 0 : retry;
    do
    {
        if (rcv_pkt != NULL)
        {
            free(rcv_pkt);
            rcv_pkt = NULL;
        }

        set_sock_timeout(sockfd, SO_SNDTIMEO);
        ret = send(sockfd, serialize(pkt), MAX_SOCKET_BUF, 0);
        if (ret < 0)
            continue;

        set_sock_timeout(sockfd, SO_RCVTIMEO);
        ret = recv(sockfd, sock_buf, MAX_SOCKET_BUF, 0);
        if (ret > 0)
        {
            rcv_pkt = deserialize((void *)sock_buf);
            if (rcv_pkt == NULL || rcv_pkt->size != pkt->size)
                continue;
        }
    } while (ret < 0 && retry-- > 0);

    if (rcv_pkt != NULL)
    {
        free(rcv_pkt);
        rcv_pkt = NULL;
    }

    return ret;
}

int settimeout(struct timeval *tmv)
{
    if (tmv == NULL)
    {
        info("[Info]: Timeout must not be NULL\n");
        return -1;
    }

    glob_tv.tv_sec = tmv->tv_sec;
    glob_tv.tv_usec = tmv->tv_usec;

    return 0;
}

void *serialize(struct packet *pkt)
{
    if (pkt == NULL)
    {
        info("[Info]: To be serialized data must not be NULL\n");
        return NULL;
    }

    pkt->seq = htons(pkt->seq);
    pkt->size = htonl(pkt->size);
    pkt->flags = htons(pkt->flags);
    for (int i = 0; i < 14; i++)
        pkt->opts[i] = htons(pkt->opts[i]);

    return (void *)pkt;
}

struct packet *deserialize(void *pkt)
{
    if (pkt == NULL)
    {
        info("[Info]: To be deserialized data must not be NULL\n");
        return NULL;
    }

    struct packet *pkt_copy, *ret_pkt;

    pkt_copy = (struct packet *)pkt;
    ret_pkt = (struct packet *)malloc(sizeof(struct packet));

    ret_pkt->seq = ntohs(pkt_copy->seq);
    ret_pkt->size = ntohl(pkt_copy->size);
    ret_pkt->flags = ntohs(pkt_copy->flags);
    for (int i = 0; i < 14; i++)
        ret_pkt->opts[i] = ntohs(pkt_copy->opts[i]);

    return ret_pkt;
}

/**
 * Sets socket timeout for the given operation.
 *
 * @param sockfd The socket file descriptor.
 * @param optname The operation.
 */
static void set_sock_timeout(int sockfd, int optname)
{
    int ret;

    ret = setsockopt(sockfd, SOL_SOCKET, optname,
                     (const char *)&glob_tv, sizeof(glob_tv));
    if (ret < 0)
    {
        if (optname == SO_SNDTIMEO)
            error("[Error]: Failed to set timeout for send op\n");
        else if (optname == SO_RCVTIMEO)
            error("[Error]: Failed to set timeout for recv op\n");
        else
            error("[Error]: Failed to set timeout for socket op\n");
        safe_exit(EXIT_FAILURE);
    }
}