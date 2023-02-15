#ifndef _SORT_H_
#define _SORT_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <sched.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>

#include "utils.h"
#include "files.h"

#define KEY_LEN 10     // first 10 bytes are the key
#define RECORD_LEN 100 // 100 bytes per record

struct sort_args
{
    uint32_t idx; // index
    uint64_t off; // file offset
    uint64_t len; // length of data to sort
    char path[30];
};

struct record
{
    unsigned char data[RECORD_LEN];
};

/**
 * Sorts a large file in the ascending order (multithreaded).
 *
 * @param ipath The input file path.
 * @param memlim The maximum memory that can be used.
 * @return 0 on success and negative on errors.
 */
int ext_sort(const char *ipath, uint32_t memlim);

/**
 * Merges sorted runs in the ascending order.
 *
 * @param ipaths The input file paths (sorted runs).
 * @param npaths The number of input files.
 * @param opath The outpu file path.
 * @param memlim The maximum memory that can be used.
 * @return 0 on success and negative on errors.
 */
int merge_sorted(const char **ipaths, uint32_t npaths, const char *opath, uint32_t memlim);

#endif