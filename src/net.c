#include "net.h"

static const struct timeval default_tmot = {
    .tv_sec = 5,
    .tv_usec = 0,
};

int init_client(const char *ip, u_short port)
{
    int handle;
    struct sockaddr_in sockaddr;

    handle = socket(AF_INET, SOCK_STREAM, 0);
    if (handle == -1)
    {
        error("[Error]: Failed to create a socket\n");
        exit(EXIT_FAILURE);
    }

    sockaddr.sin_addr.s_addr = inet_addr(ip);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);

    if (connect(handle, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
    {
        error("[Error]: Failed to connect to %s:%d\n", ip, port);
        exit(EXIT_FAILURE);
    }

    info("[Info]: Successfully created a connection to %s:%d\n", ip, port);

    return handle;
}

int init_server(u_short port)
{
    int handle;
    struct sockaddr_in sockaddr;

    handle = socket(AF_INET, SOCK_STREAM, 0);
    if (handle == -1)
    {
        error("[Error]: Failed to create a socket\n");
        exit(EXIT_FAILURE);
    }

    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);

    if (bind(handle, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
    {
        error("[Error]: Failed to bind to the port %d\n", port);
        exit(EXIT_FAILURE);
    }

    listen(handle, kQueueMax);

    info("[Info]: Successfully bound the port %d to process\n", port);
    info("[Info]: Listening to %d\n", port);

    return handle;
}

int accept_conn(int handle, struct sockaddr_in *client)
{
    int new_handle, len, client_port;
    char *client_ip;

    len = sizeof(struct sockaddr_in);
    new_handle = accept(handle, (struct sockaddr *)client, (socklen_t *)&len);
    if (new_handle < 0)
    {
        error("[Error]: Failed to accept connection\n");
        return -1;
    }

    client_ip = inet_ntoa(client->sin_addr);
    client_port = ntohs(client->sin_port);
    info("[Info]: Accepted connection to %s:%d\n", client_ip, client_port);

    return new_handle;
}

void send_data(int handle, void *data, size_t size, struct timeval *tmot)
{
    if (data == NULL)
    {
        error("[Error]: Could not send null data\n");
        return;
    }
    if (size > kDataBufferMax)
    {
        error("[Error]: Data too large (max: %dB)\n", kDataBufferMax);
        return;
    }

    int ret;
    struct timeval t;

    t.tv_sec = tmot == NULL ? default_tmot.tv_sec : tmot->tv_sec;
    t.tv_usec = tmot == NULL ? default_tmot.tv_usec : tmot->tv_usec;
    ret = setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t));
    if (ret < 0)
    {
        error("[Error]: Failed to set timeout for send operation\n");
        return;
    }

    ret = send(handle, data, size, 0);
    if (ret < 0)
    {
        error("[Error]: Failed to send\n");
    }
    else
    {
        info("[Info]: Sent %dB of data\n", ret);
    }
}

void recv_data(int handle, void *buffer, size_t size, struct timeval *tmot)
{
    if (buffer == NULL)
    {
        error("[Error]: Buffer must not be null\n");
        return;
    }
    if (size > kBufferSize)
    {
        error("[Error]: Buffer too large (max: %d)\n", kDataBufferMax);
        return;
    }

    int ret;
    struct timeval t;

    t.tv_sec = tmot == NULL ? default_tmot.tv_sec : tmot->tv_sec;
    t.tv_usec = tmot == NULL ? default_tmot.tv_usec : tmot->tv_usec;
    ret = setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
    if (ret < 0)
    {
        error("[Error]: Failed to set timeout for send operation\n");
        return;
    }

    ret = recv(handle, buffer, size, 0);
    if (ret < 0)
    {
        error("[Error]: Failed to receive\n");
    }
    else
    {
        info("[Info]: Received %dB of data\n", ret);
    }
}