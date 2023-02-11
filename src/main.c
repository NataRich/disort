#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "net.h"
#include "utils.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        error("Usage: spawn [NodeType] [Port]\n"
              "[NodeType] must be either server or client\n"
              "[Port] must be an integer beetween 1 and 65535\n");
        exit(EXIT_FAILURE);
    }
    if ((strncmp(argv[1], "server", 7) != 0) && (strncmp(argv[1], "client", 7) != 0))
    {
        error("Usage: spawn [NodeType] [Port]\n"
              "[NodeType] must be either server or client\n"
              "[Port] must be an integer beetween 1 and 65535\n");
        exit(EXIT_FAILURE);
    }
    if (atoi(argv[2]) < 1 || atoi(argv[2]) > 65535)
    {
        error("Usage: spawn [NodeType] [Port]\n"
              "[NodeType] must be either server or client\n"
              "[Port] must be an integer beetween 1 and 65535\n");
        exit(EXIT_FAILURE);
    }

    printf("Hello world\n");
    return 0;
}
