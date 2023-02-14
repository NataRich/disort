#include "proto.h"

static struct timeval glob_tv = {
    .tv_sec = 15,
    .tv_usec = 0,
};

int confirm(int sockfd, struct packet *pkt)
{
    int ret;
    struct packet *rcv_pkt;

    if (pkt == NULL)
    {
        error("[Error]: NULL meta packet\n");
        return ERR_SOCKIO; // -> close socket then retry
    }

    ret = lfm_send(sockfd, pkt);
    if (ret <= 0)
        return ERR_SOCKIO; // -> close socket then retry

    ret = lfm_recv(sockfd, &rcv_pkt);
    if (ret <= 0)
        return ERR_SOCKIO; // -> close socket then retry

    if (rcv_pkt->size != pkt->size)
    {
        free(rcv_pkt);
        return ERR_CONFIRM; // -> use the same socket and retry
    }
    if (IS_REPLY_SET(rcv_pkt->flags) != 1)
    {
        free(rcv_pkt);
        return ERR_CONFIRM; // -> use the same socket and retry
    }

    free(rcv_pkt);
    return SUCCESS;
}

int ack(int sockfd, u_int32_t *size)
{
    int ret;
    struct packet snd_pkt;
    struct packet *rcv_pkt;

    if (size == NULL)
    {
        error("[Error]: Size-holding variable must not be NULL\n");
        return ERR_SOCKIO; // -> close socket then wait
    }

    ret = lfm_recv(sockfd, &rcv_pkt);
    if (ret <= 0)
        return ERR_SOCKIO; // -> close socket then retry

    if (IS_FRST_SET(rcv_pkt->flags) != 1)
    {
        free(rcv_pkt);
        return ERR_CONFIRM; // -> close socket then wait
    }

    snd_pkt.seq = 0;
    *size = rcv_pkt->size;
    snd_pkt.size = rcv_pkt->size;
    snd_pkt.flags = LFM_F_REPLY;
    ret = lfm_send(sockfd, &snd_pkt);
    if (ret <= 0)
    {
        free(rcv_pkt);
        return ERR_SOCKIO; // -> close socket then wait
    }

    free(rcv_pkt);
    return SUCCESS;
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
        if (tmp <= 0)
            return ERR_SOCKIO; // close

        sent += tmp;
    } while (sent < MAX_SOCK_BUFFER);

    return sent;
}

ssize_t lfm_recv(int sockfd, struct packet **pkt)
{
    ssize_t rcvd;
    unsigned char buf[MAX_SOCK_BUFFER];

    // MSG_WAITALL is set, so recv() will block until buffer is full or timeout
    rcvd = recv_data(sockfd, buf, MAX_SOCK_BUFFER, &glob_tv);
    if (rcvd <= 0)
        return ERR_SOCKIO; // close
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