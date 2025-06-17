#include <sys/mman.h> # for mmap
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h> # for exit

int main(int argc, char **argv)
{
    const char *exe_name = argv[0];

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <dict_file> <seek_table_output_file>\n", exe_name);
        return EXIT_FAILURE;
    }

    const char *dict_file = argv[1];
    const char *seek_table_output_file = argv[2];

    

}