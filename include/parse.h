#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdio.h>
#include <stdlib.h>

struct node
{
    char addr[30];
    unsigned short port;
};

struct cluster
{
    struct node master;
    struct node workers[];
};

#endif