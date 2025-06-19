#include <stdlib.h>
#include <stdio.h>

#include "common.h"

FILE* fopen_orDie(const char *filename, const char *instruction)
{
    FILE* const inFile = fopen(filename, instruction);
    if (inFile) return inFile;
    /* error */
    perror(filename);
    exit(ERROR_fopen);
}

size_t fread_orDie(void *buffer, size_t sizeToRead, FILE *file)
{
    size_t const readSize = fread(buffer, 1, sizeToRead, file);
    if (readSize == sizeToRead)
        return readSize; /* good */
    if (feof(file))
        return readSize; /* good, reached end of file */
    /* error */
    perror("fread");
    exit(ERROR_fread);
}

size_t fwrite_orDie(const void* buffer, size_t sizeToWrite, FILE* file)
{
    size_t const writtenSize = fwrite(buffer, 1, sizeToWrite, file);
    if (writtenSize == sizeToWrite) return sizeToWrite;   /* good */
    /* error */
    perror("fwrite");
    exit(ERROR_fwrite);
}

int fflush_orDie(FILE *file)
{
    int const ret = fflush(file);
    if (ret == 0) return 0;
    /* error */
    perror("fflush");
    exit(ERROR_fflush);
}

void *malloc_orDie(size_t size)
{
    void *const buff = malloc(size);
    if (buff)
        return buff;
    /* error */
    perror("malloc");
    exit(ERROR_malloc);
}
