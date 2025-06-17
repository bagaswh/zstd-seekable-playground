#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"

#define ASCII_MIN 32
#define ASCII_MAX 126

static char random_word()
{
    static const char word_pool[100] = {
        
    }

    int index = rand() % word_pool_size;
    return word_pool[index];
}

int main(int argc, char **argv)
{
    const char *exe_name = argv[0];

    if (argc != 4)
    {
        fprintf(stderr, "usage: %s <filename> <num_lines> <line_size>\n", exe_name);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    int num_lines = atoi(argv[2]);
    int line_size = atoi(argv[3]);

    if (num_lines <= 0 || line_size <= 0) {
        fprintf(stderr, "Both num_lines and line_size must be positive integers.\n");
        return EXIT_FAILURE;
    }

    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand(ts.tv_nsec ^ ts.tv_sec); // XOR for slightly more randomness

    FILE *out = fopen_orDie(filename, "w");

    char *line = malloc_orDie(line_size + 1); // +1 for '\n'
    for (int i = 0; i < num_lines; i++) {
        for (int j = 0; j < line_size; j++) {
            line[j] = random_printable_char();
        }
        line[line_size] = '\n';
        fwrite_orDie(line, line_size + 1, out);
    }

    free(line);
    fclose(out);
    return EXIT_SUCCESS;
}
