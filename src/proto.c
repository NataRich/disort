#include "proto.h"

static struct timeval glob_tv = {
    .tv_sec = 15,
    .tv_usec = 0,
};

int confirm(int sockfd, struct packet *pkt, short retry)
{
    int ret;
    struct packet *rcv_pkt;
    char sock_buf[MAX_SOCK_BUFFER];

    if (pkt == NULL)
    {
        error("[Error]: File meta packet must not be NULL\n");
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

        ret = send_data(sockfd, serialize(pkt), MAX_SOCK_BUFFER, &glob_tv);
        if (ret < 0)
            continue;

        ret = recv_data(sockfd, sock_buf, MAX_SOCK_BUFFER, &glob_tv);
        if (ret > 0)
        {
            rcv_pkt = deserialize((void *)sock_buf);
            if (rcv_pkt == NULL)
                continue;
            if (rcv_pkt->size != pkt->size)
                continue;
            if (IS_REPLY_SET(rcv_pkt->flags) != 1)
                continue;
        }
    } while (ret < 0 && retry-- > 0);

    if (rcv_pkt != NULL)
    {
        free(rcv_pkt);
        rcv_pkt = NULL;
    }

    if (ret < 0)
        error("[Error]: Failed to confirm transfer\n");

    return ret;
}

int reply(int sockfd, u_int32_t *size)
{
    int ret;
    struct packet snd_pkt;
    struct packet *rcv_pkt;
    char sock_buf[MAX_SOCK_BUFFER];

    if (size == NULL)
    {
        error("[Error]: Size-holding variable must not be NULL\n");
        return -1;
    }

    ret = recv_data(sockfd, sock_buf, MAX_SOCK_BUFFER, &glob_tv);
    if (ret < 0)
    {
        error("[Error]: Failed to receive a confirmation\n");
        return -1;
    }

    rcv_pkt = deserialize((void *)sock_buf);
    if (rcv_pkt == NULL)
        return -1;

    if (IS_FRST_SET(rcv_pkt->flags) != 1)
    {
        error("[Error]: Received packet does not have LFM_F_FRST set\n");
        return -1;
    }

    snd_pkt.seq = 0;
    *size = rcv_pkt->size;
    snd_pkt.size = rcv_pkt->size;
    snd_pkt.flags = LFM_F_REPLY;
    ret = send_data(sockfd, &snd_pkt, MAX_SOCK_BUFFER, &glob_tv);
    if (ret < 0)
    {
        error("[Error]: Failed to send a reply.");
        return -1;
    }

    return ret;
}

ssize_t lfm_send(int sockfd, struct packet *pkt)
{
    void *raw;
    ssize_t sent, tmp;
    struct packet copy;

    memcpy(&copy, pkt, sizeof(struct packet));

    sent = 0;
    raw = serialize(&copy);
    do
    {
        tmp = send_data(sockfd, raw + sent, MAX_SOCK_BUFFER - sent, &glob_tv);
        if (tmp < 0)
        {
            tmp = 0;
            info("[Info]: Failed to send the packet (%d, %d, %d)"
                 " at offset %d of length %d (Retrying...)\n",
                 pkt->seq, pkt->size,
                 pkt->flags, sent, MAX_SOCK_BUFFER - sent);
            usleep(500 * 1000); // wait for half a second
        }

        sent += tmp;
    } while (sent < MAX_SOCK_BUFFER);

    return sent;
}

ssize_t lfm_recv(int sockfd, struct packet **pkt)
{
    int flag;
    ssize_t rcvd, tmp;
    unsigned char buf[MAX_SOCK_BUFFER];

    rcvd = flag = 0;
    do
    {
        // TODO: should not need loops anymore when MSG_WAITALL flag is set on recv()
        tmp = recv_data(sockfd, buf + rcvd, MAX_SOCK_BUFFER - rcvd, &glob_tv);
        if (tmp < 0)
        {
            tmp = 0;
            info("[Info]: Failed to receive the last packet at offset %ld"
                 " with length %ld (Retrying...)\n",
                 rcvd, MAX_SOCK_BUFFER - rcvd);
            usleep(500 * 1000); // wait for half a second
        }
        else if (tmp == 0)
        {
            flag = 1;
            info("[Info]: End of stream (stopped receiving)\n");
            break;
        }

        flag = 0;
        rcvd += tmp;
    } while (rcvd < MAX_SOCK_BUFFER);

    if (flag == 1 && rcvd < MAX_SOCK_BUFFER)
        return rcvd;

    *pkt = deserialize((void *)buf);
    return rcvd;
}

int settimeout(struct timeval *tmv)
{
    if (tmv == NULL)
    {
        error("[Error]: Timeout must not be NULL\n");
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
        error("[Error]: To be serialized data must not be NULL\n");
        return NULL;
    }

    pkt->seq = htons(pkt->seq);
    pkt->size = htonl(pkt->size);
    pkt->flags = htons(pkt->flags);
    for (int i = 0; i < PACKET_PAD; i++)
        pkt->opts[i] = htons(pkt->opts[i]);

    return (void *)pkt;
}

struct packet *deserialize(void *pkt)
{
    if (pkt == NULL)
    {
        error("[Error]: To be deserialized data must not be NULL\n");
        return NULL;
    }

    struct packet *pkt_copy, *ret_pkt;

    pkt_copy = (struct packet *)pkt;
    ret_pkt = (struct packet *)malloc(sizeof(struct packet));

    ret_pkt->seq = ntohs(pkt_copy->seq);
    ret_pkt->size = ntohl(pkt_copy->size);
    ret_pkt->flags = ntohs(pkt_copy->flags);
    for (int i = 0; i < PACKET_PAD; i++)
        ret_pkt->opts[i] = ntohs(pkt_copy->opts[i]);
    memcpy(ret_pkt->buf, pkt_copy->buf, MAX_PACKET_BUF);

    return ret_pkt;
}