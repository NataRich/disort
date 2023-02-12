#include "utils.h"

int is_child()
{
    pid_t pid = getpid();
    pid_t ppid = getppid();
    return pid == ppid ? 1 : 0;
}

void safe_exit(int status)
{
    if (is_child() == 0)
        _exit(status);
    else
        exit(status);
}

void error(const char *format, ...)
{
    va_list args;
    char buffer[MAX_BUFFER_SIZE] = {0};

    va_start(args, format);
    vsnprintf(buffer, MAX_BUFFER_SIZE, format, args);
    va_end(args);

    write(STDERR_FILENO, buffer, strlen(buffer));
}

void info(const char *format, ...)
{
    va_list args;
    char buffer[MAX_BUFFER_SIZE] = {0};

    va_start(args, format);
    vsnprintf(buffer, MAX_BUFFER_SIZE, format, args);
    va_end(args);

    write(STDOUT_FILENO, buffer, strlen(buffer));
}