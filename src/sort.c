#include "sort.h"

static uint32_t get_cpus()
{
    cpu_set_t cs;
    sched_getaffinity(0, sizeof(cs), &cs);
    return CPU_COUNT_S(sizeof(cs), &cs);
}

static int compar(const void *a, const void *b)
{
    struct record *ra = (struct record *)a;
    struct record *rb = (struct record *)b;
    return memcmp(ra->data, rb->data, KEY_LEN);
}

static void *sort_routine(void *args)
{
    pid_t self;
    FILE *ifp, *ofp;
    uint32_t i, nrecords;
    struct record *records;
    struct sort_args *sargs;
    char opath[50];

    self = syscall(__NR_gettid);
    sargs = (struct sort_args *)args;
    nrecords = sargs->len / RECORD_LEN;

    // read unordered data from file
    ifp = fopen(sargs->path, "rb");
    if (ifp == NULL)
    {
        error("[Error][t%d]: fopen() failed\n", self);
        free(args);
        return NULL;
    }

    if (fseek(ifp, sargs->off, SEEK_SET) < 0)
    {
        error("[Error][t%d]: fseek() failed\n", self);
        fclose(ifp);
        free(args);
        return NULL;
    }

    records = (struct record *)malloc(nrecords * sizeof(struct record));
    if (records == NULL)
    {
        error("[Error][t%d]: malloc() failed\n", self);
        fclose(ifp);
        free(args);
        return NULL;
    }
    for (i = 0; i < nrecords; i++)
        fread(records[i].data, RECORD_LEN, 1, ifp); // assume no errors
    fclose(ifp);

    // in-memory quick sort
    qsort(records, nrecords, RECORD_LEN, compar);

    // write sorted run to file
    sprintf(opath, "%s.%d.sorted", sargs->path, sargs->idx);
    if (fcreate(opath) < 0)
    {
        error("[Error][t%d]: fcreate() failed %s\n", self, opath);
        free(records);
        free(args);
        return NULL;
    }

    ofp = fopen(opath, "ab");
    for (i = 0; i < nrecords; i++)
        fwrite(records[i].data, RECORD_LEN, 1, ofp); // assume no errors

    fclose(ofp);
    free(records);
    free(args);
    return NULL;
}

int ext_sort(const char *ipath, uint32_t memlim)
{
    long fsize;
    pthread_t *tids;
    uint32_t idx, nthreads, chunk_size;

    nthreads = get_cpus() * 2;
    tids = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
    if (tids == NULL)
    {
        error("[Error]: malloc() failed during sort\n");
        safe_exit(EXIT_FAILURE);
    }

    fsize = fgetsize(ipath);
    if (fsize < 0)
    {
        error("[Error]: fgetsize() failed\n");
        safe_exit(EXIT_FAILURE);
    }

    // generate sorted runs
    idx = 0;
    chunk_size = (memlim / nthreads / RECORD_LEN) * RECORD_LEN;
    while (idx * chunk_size < fsize)
    {
        uint32_t i, stop;

        stop = nthreads;
        for (i = 0; i < nthreads; i++)
        {
            long diff;
            struct sort_args *args;

            diff = fsize - idx * chunk_size;
            if (diff <= 0)
            {
                stop = i;
                break;
            }

            args = (struct sort_args *)malloc(sizeof(struct sort_args));
            if (args == NULL)
            {
                error("[Error]: malloc() failed during sort\n");
                safe_exit(EXIT_FAILURE);
            }
            args->idx = idx;
            args->off = idx * chunk_size;
            args->len = diff >= chunk_size ? chunk_size : diff;
            memcpy(args->path, ipath, 30);
            pthread_create(&tids[i], NULL, sort_routine, args);

            idx++;
        }

        for (i = 0; i < stop; i++)
            pthread_join(tids[i], NULL);
    }

    info("[Info]: Created %d sorted files\n", idx);
    free(tids);
    return 0;
}

static uint32_t first_of(const struct record *records, uint32_t nrecords)
{
    uint32_t i, idx;
    struct record *copy;

    copy = (struct record *)malloc(nrecords * sizeof(struct record)); // assume no errors
    memcpy(copy, records, RECORD_LEN * nrecords);
    qsort(copy, nrecords, RECORD_LEN, compar);
    for (i = 0; i < nrecords; i++)
    {
        if (memcmp(&copy[0], &records[i], KEY_LEN) == 0)
        {
            idx = i;
            break;
        }
    }

    free(copy);
    return idx;
}

int merge_sorted(const char **ipaths, uint32_t npaths, const char *opath, uint32_t memlim)
{
    return 0;
}