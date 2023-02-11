#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "net.h"
#include "utils.h"

void print_usage_and_exit();

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        print_usage_and_exit();
    }
    if ((strncmp(argv[1], "server", 7) != 0) && (strncmp(argv[1], "client", 7) != 0))
    {
        print_usage_and_exit();
    }
    if (atoi(argv[2]) < 1 || atoi(argv[2]) > 65535)
    {
        print_usage_and_exit();
    }

    printf("Hello world\n");
    return 0;
}

void print_usage_and_exit()
{
    error("Usage: spawn [NodeType] [Port]\n"
          "[NodeType] must be either server or client\n"
          "[Port] must be an integer beetween 1 and 65535\n");
    exit(EXIT_FAILURE);
}