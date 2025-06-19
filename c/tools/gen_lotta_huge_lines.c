#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"

#define ASCII_MIN 65
#define ASCII_MAX 90

char random_alphabetic_char(void)
{
    int index = rand() % 26;
    return 'A'+index;
}

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0) /* record sign */
        n = -n;         /* make n positive */
    i = 0;
    do
    {                          /* generate digits in reverse order */
        s[i++] = n % 10 + '0'; /* get next digit */
    } while ((n /= 10) > 0); /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);

    return i;
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

    if (num_lines <= 0 || line_size <= 0)
    {
        fprintf(stderr, "Both num_lines and line_size must be positive integers.\n");
        return EXIT_FAILURE;
    }

    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand(ts.tv_nsec ^ ts.tv_sec);

    FILE *out = fopen_orDie(filename, "w");

    char *line = malloc_orDie(line_size);
    for (int i = 0; i < num_lines; i++)
    {
        int idx = itoa(i, line);
        for (int j = idx; j < line_size - 1; j++)
        {
            line[j] = random_alphabetic_char();
        }
        line[line_size - 1] = '\n';
        fwrite_orDie(line, line_size, out);
    }
    fflush_orDie(out);

    free(line);
    fclose(out);
    return EXIT_SUCCESS;
}
