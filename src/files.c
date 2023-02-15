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

long fgetsize(const char *path)
{
    long ret;
    FILE *fp;

    fp = fopen(path, "rb");
    if (fp == NULL)
        return -1;
    if (fseek(fp, 0, SEEK_END) < 0)
        return -1;

    ret = ftell(fp);
    fclose(fp);
    return ret;
}

int ftransfer(int sockfd, char *path)
{
    int ret, npkt;
    FILE *fp;
    u_int16_t seqno;
    struct packet pkt;
    ssize_t fsize, sent, off;
    unsigned char data_buf[MAX_PACKET_BUF];

    if (fexists(path) != 0)
    {
        error("[Error]: %s does not exist\n", path);
        return ERR_FILEIO;
    }

    fp = fopen(path, "rb");
    if (fp == NULL)
    {
        error("[Error]: Failed to fopen %s\n", path);
        return ERR_FILEIO;
    }

    if (fseek(fp, 0, SEEK_END) < 0)
    {
        error("[Error]: Failed to fseek %s (SEEK_END)\n", path);
        return ERR_FILEIO;
    }

    fsize = ftell(fp);
    if (fsize < 0)
    {
        error("[Error]: Failed to ftell %s\n", path);
        return ERR_FILEIO;
    }

    info("[Info]: %s is %dB in size\n", path, fsize);

    seqno = 0;

    // Confirm phase
    info("[Info]: Confirming...\n");
    pkt.seq = seqno++;
    pkt.size = (u_int32_t)fsize;
    pkt.flags = LFM_F_FRST;
    memset(pkt.buf, 0, MAX_PACKET_BUF);
    ret = confirm(sockfd, &pkt);
    if (ret != SUCCESS)
        return ret;

    // Transfer phase
    info("[Info]: Transfering...\n");
    if (fseek(fp, 0, SEEK_SET) < 0)
    {
        error("[Error]: Failed to seek %s (SEEK_SET)\n", path);
        return ERR_FILEIO;
    }

    off = npkt = 0;
    do
    {
        npkt++;
        sent = 0;

        pkt.seq = seqno++;
        if (off + MAX_PACKET_BUF > fsize) // last part
        {
            pkt.size = fsize - off;
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
        if (sent == ERR_SOCKIO)
        {
            fclose(fp);
            return sent; // -> close socket and start over
        }
    } while (off < fsize);

    info("[Info]: Sent=%ldB via %d packets\n", fsize, npkt);
    fclose(fp);
    return SUCCESS;
}

int freceive(int sockfd, char *path)
{
    int ret;
    FILE *fp;
    ssize_t rcvd, trcvd;
    u_int32_t fsize;
    struct packet *pkt;

    if (path == NULL)
    {
        error("[Error]: Path must not be NULL\n");
        return ERR_FILEIO;
    }

    // Acknowledge phase
    ret = ack(sockfd, &fsize);
    if (ret != SUCCESS)
        return ret;

    info("[Info]: Expect to receive %dB\n", fsize);

    if (fcreate(path) < 0)
    {
        error("[Error]: Failed to create %s\n", path);
        return ERR_FILEIO;
    }

    fp = fopen(path, "ab");
    if (fp == NULL)
    {
        error("[Error]: Failed to open %s\n", path);
        return ERR_FILEIO;
    }

    // Receive phase
    trcvd = 0;
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
        if (rcvd == ERR_SOCKIO || IS_PART_SET(pkt->flags) != 1)
        {
            fclose(fp);
            return ERR_SOCKIO; // -> close socket and wait
        }

        trcvd += pkt->size;
        if (fwrite(pkt->buf, pkt->size, 1, fp) < 1)
        {
            error("[Error]: Failed to fwrite\n");
            free(pkt);
            fclose(fp);
            return ERR_FILEIO;
        }
    } while (IS_LAST_SET(pkt->flags) != 1);

    info("[Info]: Wrote %ldB (exp %ldB) in %s\n", trcvd, fsize, path);
    free(pkt);
    fclose(fp);
    return trcvd == fsize ? SUCCESS : ERR_PARTIAL; // -> use the same socket and start over (wait)
}
