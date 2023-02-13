#include "net.h"

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

ssize_t send_data(int sockfd, void *data, size_t size, struct timeval *tv)
{
    if (data == NULL)
    {
        error("[Error]: Could not send null data\n");
        return -1;
    }
    if (size > MAX_SOCK_BUFFER)
    {
        error("[Error]: Data too large (max: %dB)\n", MAX_SOCK_BUFFER);
        return -1;
    }
    if (tv == NULL)
    {
        error("[Error]: Timeout must notbe NULL\n");
        return -1;
    }

    if (set_sock_timeout(sockfd, SO_SNDTIMEO, tv) < 0)
        return -1;
    return send(sockfd, data, size, 0);
}

ssize_t recv_data(int sockfd, void *buffer, size_t size, struct timeval *tv)
{
    if (buffer == NULL)
    {
        error("[Error]: Buffer must not be null\n");
        return -1;
    }
    if (size > MAX_SOCK_BUFFER)
    {
        error("[Error]: Buffer too large (max: %d)\n", MAX_SOCK_BUFFER);
        return -1;
    }
    if (tv == NULL)
    {
        error("[Error]: Timeout must notbe NULL\n");
        return -1;
    }

    if (set_sock_timeout(sockfd, SO_RCVTIMEO, tv) < 0)
        return -1;
    return recv(sockfd, buffer, size, MSG_WAITALL);
}

int set_sock_timeout(int sockfd, int optname, struct timeval *tv)
{
    int ret, len;

    if (tv == NULL)
    {
        error("[Error]: Timeout must not be NULL\n");
        return -1;
    }

    len = sizeof(struct timeval);
    ret = setsockopt(sockfd, SOL_SOCKET, optname, (const char *)tv, len);
    if (ret < 0)
    {
        if (optname == SO_SNDTIMEO)
            error("[Error]: Failed to set timeout for send op\n");
        else if (optname == SO_RCVTIMEO)
            error("[Error]: Failed to set timeout for recv op\n");
        else
            error("[Error]: Failed to set timeout for socket op\n");
        return -1;
    }

    return 0;
}