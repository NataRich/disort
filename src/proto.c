#include "proto.h"

static struct timeval glob_tv = {
    .tv_sec = 15,
    .tv_usec = 0,
};

int confirm(int sockfd, struct packet *pkt, short retry)
{
    int ret;
    struct packet rcv_pkt;

    if (pkt == NULL)
    {
        info("[Info]: File meta packet must not be null\n");
        return -1;
    }

    retry = retry < 0 ? 0 : retry;
    do
    {
        set_sock_timeout(sockfd, SO_SNDTIMEO);
        ret = send(sockfd, serialize(pkt), MAX_SOCKET_BUF, 0);
        if (ret < 0)
            continue;

        set_sock_timeout(sockfd, SO_RCVTIMEO);
        ret = recv(sockfd, &rcv_pkt, MAX_SOCKET_BUF, 0);
    } while (ret < 0 && retry-- > 0);

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

    struct packet *pkt_copy;

    pkt_copy = (struct packet *)pkt;

    pkt_copy->seq = ntohs(pkt_copy->seq);
    pkt_copy->size = ntohl(pkt_copy->size);
    pkt_copy->flags = ntohs(pkt_copy->flags);
    for (int i = 0; i < 14; i++)
        pkt_copy->opts[i] = ntohs(pkt_copy->opts[i]);

    return (void *)pkt;
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