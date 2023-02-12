#include "net.h"

static const struct timeval default_tv = {
    .tv_sec = 5,
    .tv_usec = 0,
};

int init_client(const char *ip, u_short port)
{
    int sockfd;
    struct sockaddr_in sockaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        error("[Error]: Failed to create a stream socket\n");
        safe_exit(EXIT_FAILURE);
    }

    sockaddr.sin_addr.s_addr = inet_addr(ip);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
    {
        error("[Error]: Failed to connect to %s:%d\n", ip, port);
        safe_exit(EXIT_FAILURE);
    }

    info("[Info]: Successfully connected to %s:%d\n", ip, port);

    return sockfd;
}

int init_server(u_short port)
{
    int sockfd;
    struct sockaddr_in sockaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        error("[Error]: Failed to create a socket\n");
        safe_exit(EXIT_FAILURE);
    }

    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
    {
        error("[Error]: Failed to bind to the port %d to socket\n", port);
        safe_exit(EXIT_FAILURE);
    }

    if (listen(sockfd, MAX_SOCK_QUEUE) < 0)
    {
        error("[Error]: Failed to prepare to accept connections\n");
        safe_exit(EXIT_FAILURE);
    }

    info("[Info]: Listening on %d\n", port);

    return sockfd;
}

int accept_conn(int sockfd, struct sockaddr_in *client)
{
    int nsockfd, len, client_port;
    char *client_ip;

    len = sizeof(struct sockaddr_in);
    nsockfd = accept(sockfd, (struct sockaddr *)client, (socklen_t *)&len);
    if (nsockfd < 0)
    {
        error("[Error]: Failed to accept connection\n");
        return -1;
    }

    client_ip = inet_ntoa(client->sin_addr);
    client_port = ntohs(client->sin_port);

    info("[Info]: Successfully connected to %s:%d\n", client_ip, client_port);

    return nsockfd;
}

void send_data(int sockfd, void *data, size_t size, struct timeval *tmot)
{
    int ret;
    struct timeval tv;

    if (data == NULL)
    {
        error("[Error]: Could not send null data\n");
        return;
    }
    if (size > MAX_SOCK_BUFFER)
    {
        error("[Error]: Data too large (max: %dB)\n", MAX_SOCK_BUFFER);
        return;
    }

    tv.tv_sec = tmot == NULL ? default_tv.tv_sec : tmot->tv_sec;
    tv.tv_usec = tmot == NULL ? default_tv.tv_usec : tmot->tv_usec;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    if (ret < 0)
    {
        error("[Error]: Failed to set timeout for send operation\n");
        return;
    }

    ret = send(sockfd, data, size, 0);
    if (ret < 0)
        error("[Error]: Failed to send\n");
    else
        info("[Info]: Sent %dB of data\n", ret);
}

void recv_data(int handle, void *buffer, size_t size, struct timeval *tmot)
{
    int ret;
    struct timeval tv;

    if (buffer == NULL)
    {
        error("[Error]: Buffer must not be null\n");
        return;
    }
    if (size > MAX_SOCK_BUFFER)
    {
        error("[Error]: Buffer too large (max: %d)\n", MAX_SOCK_BUFFER);
        return;
    }

    tv.tv_sec = tmot == NULL ? default_tv.tv_sec : tmot->tv_sec;
    tv.tv_usec = tmot == NULL ? default_tv.tv_usec : tmot->tv_usec;
    ret = setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
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