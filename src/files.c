#include "files.h"

int fexists(char *path)
{
    return access(path, F_OK);
}

int fcreate(char *path)
{
    FILE *fp = fopen(path, "w");
    if (fp == NULL)
        return -1;
    fclose(fp);
    return 0;
}

int ftransfer(int sockfd, char *path)
{
    FILE *fp;
    u_int16_t seqno;
    struct packet pkt;
    ssize_t size, sent, off;
    unsigned char data_buf[MAX_PACKET_BUF];

    if (fexists(path) != 0)
    {
        error("[Error]: File (%s) does not exist\n", path);
        return -1;
    }

    fp = fopen(path, "rb");
    if (fp == NULL)
    {
        error("[Error]: Failed to open the file (%s)\n", path);
        return -1;
    }

    if (fseek(fp, 0, SEEK_END) < 0)
    {
        error("[Error]: Failed to seek the end of the file (%s)\n", path);
        return -1;
    }

    size = ftell(fp);
    if (size < 0)
    {
        error("[Error]: Failed to ftell (%s)\n", path);
        return -1;
    }

    info("[Info]: File (%s) size is %d\n", path, size);

    seqno = 0;

    // Confirm phase
    info("[Info]: Started confirming...\n");
    pkt.seq = seqno++;
    pkt.size = (u_int32_t)size;
    pkt.flags = LFM_F_FRST;
    memset(pkt.buf, 0, MAX_PACKET_BUF);
    if (confirm(sockfd, &pkt, 3) < 0)
        return -1;

    // Transfer phase
    info("[Info]: Confirmation succeeded. Started transfering...\n");
    off = 0;
    do
    {
        sent = 0;

        if (fseek(fp, off, SEEK_SET) < 0)
        {
            error("[Error]: Failed to seek the end of the file (%s)\n", path);
            return -1;
        }

        pkt.seq = seqno++;
        if (off + MAX_PACKET_BUF > size) // last part
        {
            pkt.size = size - off;
            pkt.flags = LFM_F_PART | LFM_F_LAST;
        }
        else
        {
            pkt.size = MAX_PACKET_BUF;
            pkt.flags = LFM_F_PART;
        }

        off += pkt.size;

        fread(data_buf, pkt.size, 1, fp);
        memcpy(pkt.buf, data_buf, pkt.size);
        sent = lfm_send(sockfd, &pkt);

        info("[Info]: Sent %dB of data (%dB real)\n", sent, pkt.size);
    } while (off < size);

    info("[Info]: File (%s) transfer completed.\n", path);
    fclose(fp);
    return 0;
}

int freceive(int sockfd, char *path)
{
    FILE *fp;
    ssize_t rcvd;
    u_int32_t fsize;
    struct packet *pkt;

    if (path == NULL)
    {
        error("[Error]: Path must not be NULL\n");
        return -1;
    }

    // Reply phase
    if (reply(sockfd, &fsize) < 0)
        return -1;

    info("[Info]: Expect to receive %dB of data\n", fsize);

    if (fcreate(path) < 0)
    {
        error("[Error]: Failed to create a file for writing\n");
        return -1;
    }

    fp = fopen(path, "ab");
    if (fp == NULL)
    {
        error("[Error]: Failed to open a file for writing\n");
        return -1;
    }

    // Receive phase
    pkt = NULL;
    do
    {
        rcvd = 0;

        if (pkt != NULL)
        {
            free(pkt);
            pkt = NULL;
        }

        rcvd = lfm_recv(sockfd, &pkt);
        if (rcvd < MAX_SOCK_BUFFER)
        {
            // TODO: initiate retry the entire file
            error("[Error]: Incomplete packet received\n");
            fclose(fp);
            return -1;
        }

        fwrite(pkt->buf, pkt->size, 1, fp);

        info("[Info]: Received %dB of data (%dB real)\n", rcvd, pkt->size);
    } while (IS_LAST_SET(pkt->flags) != 1);

    if (pkt != NULL)
        free(pkt);

    info("[Info]: Data fully persisted in %s\n", path);
    fclose(fp);
    return 0;
}
