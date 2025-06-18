#define _POSIX_C_SOURCE 200809L

#include <argp.h>
#include <stdlib.h>
#include <stdio.h>
#include <bits/stdint-uintn.h>
#include <bits/stdint-intn.h>
#include <sys/mman.h> // for mmap
#include <unistd.h>   // for getpagesize
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // for fstat
#include <inttypes.h>

#include "zstd.h"
#include "zstd_seekable.h"
#include "common.h"
#include "string_view.h"

const char *argp_program_version = "zstd-seekable-compress 0.1";
const char *argp_program_bug_address = "<you@example.com>";
static char doc[] = "zstd-seekable-compress — compress files in seekable ZSTD format";
static char args_doc[] = "";

static struct argp_option options[] = {
    {"file", 'f', "FILE", 0, "Input file (default: stdin)", 0},
    {"output", 'o', "FILE", 0, "Output file (default: stdout)", 0},
    {"compression level", 'l', "INT", 0, "Compression level (default: 10)", 0},
    {"frame size", 's', "INT", 0, "Frame size (default: 65536)", 0},
    {"verbose", 'v', 0, 0, "Enable verbose output", 1},
    {0}};

struct arguments
{
    char *input_file;
    char *output_file;
    int verbose;
    int compression_level;
    unsigned int frame_size;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
    case 'f':
        arguments->input_file = arg;
        break;
    case 'o':
        arguments->output_file = arg;
        break;
    case 'l':
        arguments->compression_level = atoi(arg);
        break;
    case 's':
        arguments->frame_size = atoi(arg);
        break;
    case 'v':
        arguments->verbose = 1;
        break;
    case ARGP_KEY_ARG:
        // Reject unexpected positional args
        argp_usage(state);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, .children = NULL};

#define ERR_OK 0
#define ERR_ALLOC 1
#define ERR_ZSTD_INIT 2
#define ERR_ZSTD_COMPRESS 3

typedef struct
{
    uint32_t Number_Of_Frames;
    uint8_t Seek_Table_Descriptor;
    uint32_t Seekable_Magic_Number;
} __attribute__((packed)) seek_table_footer;

typedef struct
{
    uint32_t Compressed_Size;
    uint32_t Decompressed_Size;
    uint32_t Checksum;
} __attribute__((packed)) seek_table_entry;

#define SEEK_TABLE_FOOTER_SIZE 9
#define SEEK_TABLE_ENTRY_SIZE 12

void str_print_slice(char *ptr, size_t start_pos, size_t len)
{
    fwrite(ptr + start_pos, 1, len, stdout);
}

int zstd_seekable_print_info(FILE *in_compressed, FILE *in_decompressed)
{
    // fseek(in, -(int)SEEK_TABLE_FOOTER_SIZE, SEEK_END);
    // seek_table_footer footer;
    // fread(&footer, sizeof(footer), 1, in);

    // printf("------------\n");
    // printf("Number of frames: %d\n", footer.Number_Of_Frames);
    // printf("Seek table descriptor: %d\n", footer.Seek_Table_Descriptor);
    // printf("Seekable magic number: %d\n", footer.Seekable_Magic_Number);

    // seek_table_entry *entries = malloc(footer.Number_Of_Frames * sizeof(seek_table_entry));
    // fseek(in, -(9 + footer.Number_Of_Frames * sizeof(seek_table_entry)), SEEK_END);
    // fread(entries, sizeof(seek_table_entry), footer.Number_Of_Frames, in);

    // printf("\n");
    // for (uint32_t i = 0; i < footer.Number_Of_Frames; i++)
    // {
    //     printf("Reading entry %d\n", i);
    //     printf("Compressed size: %d\n", entries[i].Compressed_Size);
    //     printf("Decompressed size: %d\n", entries[i].Decompressed_Size);
    //     printf("Checksum: %d\n", entries[i].Checksum);
    //     printf("------------\n");
    // }

    // free(entries);

    ZSTD_seekable *zs = ZSTD_seekable_create();
    if (zs == NULL)
    {
        ZSTD_seekable_free(zs);
        fprintf(stderr, "ZSTD_seekable_create() error \n");
        return ERR_ZSTD_INIT;
    }
    size_t const init_ret = ZSTD_seekable_initFile(zs, in_compressed);
    if (ZSTD_isError(init_ret))
    {
        ZSTD_seekable_free(zs);
        fprintf(stderr, "ZSTD_seekable_initFile() error : %s \n", ZSTD_getErrorName(init_ret));
        return ERR_ZSTD_INIT;
    }
    ZSTD_seekTable *st = ZSTD_seekTable_create_fromSeekable(zs);
    if (st == NULL)
    {
        ZSTD_seekable_free(zs);
        ZSTD_seekTable_free(st);
        fprintf(stderr, "ZSTD_seekTable_create_fromSeekable() error \n");
        return ERR_ZSTD_INIT;
    }

    size_t const num_frames = ZSTD_seekTable_getNumFrames(st);
    printf("Number of frames: %zu\n", num_frames);

    struct stat file_stat_compressed;
    int fstat_rc = fstat(fileno(in_compressed), &file_stat_compressed);
    if (fstat_rc != 0)
    {
        perror("fstat");
        return 1;
    }

    char *ptr_compressed = mmap(NULL, file_stat_compressed.st_size, PROT_READ, MAP_PRIVATE, fileno(in_compressed), 0);
    if (ptr_compressed == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    struct stat file_stat_decompressed;
    fstat_rc = fstat(fileno(in_decompressed), &file_stat_decompressed);
    if (fstat_rc != 0)
    {
        perror("fstat");
        return 1;
    }

    char *ptr_decompressed = mmap(NULL, file_stat_decompressed.st_size, PROT_READ, MAP_PRIVATE, fileno(in_decompressed), 0);
    if (ptr_decompressed == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    for (uint32_t i = 0; i < num_frames; i++)
    {
        printf("Reading entry %d\n", i);

        size_t const compressed_size = ZSTD_seekTable_getFrameCompressedSize(st, i);
        printf("Compressed size: %zu\n", compressed_size);
        size_t const compressed_start_pos = ZSTD_seekTable_getFrameCompressedOffset(st, i);
        printf("Compressed offset: %zu\n", compressed_start_pos);
        printf("Compressed printed:\n");
        str_print_slice(ptr_compressed, compressed_start_pos, compressed_size);
        printf("\n");

        size_t const decompressed_size = ZSTD_seekTable_getFrameDecompressedSize(st, i);
        printf("Decompressed size: %zu\n", decompressed_size);
        size_t const decompressed_start_pos = ZSTD_seekTable_getFrameDecompressedOffset(st, i);
        printf("Decompressed offset: %zu\n", decompressed_start_pos);
        printf("Decompressed printed:\n");
        str_print_slice(ptr_decompressed, decompressed_start_pos, decompressed_size);
        printf("\n");

        // printf("Checksum: %d\n", ZSTD_seekTable_getFrameChecksum(st, i));
        printf("------------\n");
    }

    ZSTD_seekTable_free(st);
    ZSTD_seekable_free(zs);

    return 0;
}

int do_compress(FILE *in, FILE *out, int compression_level, unsigned int frame_size)
{
    int ret = ERR_OK;

    ZSTD_seekable_CStream *zcs = ZSTD_seekable_createCStream();
    if (zcs == NULL)
    {
        return ERR_ZSTD_INIT;
    }

    size_t const buff_in_size = ZSTD_CStreamInSize();
    void *const buff_in = malloc_orDie(buff_in_size);
    size_t const buff_out_size = ZSTD_CStreamOutSize();
    void *const buff_out = malloc_orDie(buff_out_size);

    size_t const init_result = ZSTD_seekable_initCStream(zcs, compression_level, 1, frame_size);
    if (ZSTD_isError(init_result))
    {
        fprintf(stderr, "ZSTD_seekable_initCStream() error : %s \n", ZSTD_getErrorName(init_result));
        exit(11);
    }

    size_t read, to_read = buff_in_size;
    while ((read = fread_orDie(buff_in, to_read, in)))
    {
        ZSTD_inBuffer input = {buff_in, read, 0};
        while (input.pos < input.size)
        {
            ZSTD_outBuffer output = {buff_out, buff_out_size, 0};
            to_read = ZSTD_seekable_compressStream(zcs, &output, &input);
            if (ZSTD_isError(to_read))
            {
                fprintf(stderr, "ZSTD_seekable_compressStream() error : %s \n", ZSTD_getErrorName(to_read));
                exit(12);
            }
            if (to_read > buff_in_size)
                to_read = buff_in_size;
            fwrite_orDie(buff_out, output.pos, out);
        }
    }

    while (1)
    {
        ZSTD_outBuffer output = {buff_out, buff_out_size, 0};
        size_t const remaining_to_flush = ZSTD_seekable_endStream(zcs, &output);
        CHECK_ZSTD(remaining_to_flush);
        fwrite_orDie(buff_out, output.pos, out);
        if (!remaining_to_flush)
            break;
    }

    ZSTD_seekable_freeCStream(zcs);
    free(buff_in);
    free(buff_out);

    return ret;
}

int main(int argc, char **argv)
{
    struct arguments args = {
        .input_file = NULL,
        .output_file = NULL,
        .compression_level = 10,
        .frame_size = 0,
        .verbose = 0};

    argp_parse(&argp, argc, argv, 0, 0, &args);

    if (args.frame_size == 0)
    {
        args.frame_size = 65536;
    }

    if (args.verbose)
    {
        printf("Verbose mode enabled\n");
        printf("Input:  %s\n", args.input_file ? args.input_file : "stdin");
        printf("Output: %s\n", args.output_file ? args.output_file : "stdout");
        printf("Frame size: %d\n", args.frame_size);
        printf("Compression level: %d\n", args.compression_level);
    }

    // Example: open files or use stdin/stdout
    FILE *in = args.input_file ? fopen(args.input_file, "rb") : stdin;
    FILE *out = args.output_file ? fopen(args.output_file, "wb") : stdout;

    if (!in)
    {
        perror("Failed to open input file");
        return 1;
    }
    if (!out)
    {
        perror("Failed to open output file");
        if (in != stdin)
            fclose(in);
        return 1;
    }

    int ret = do_compress(in, out, args.compression_level, args.frame_size);

    if (in != stdin)
        fclose(in);
    if (out != stdout)
        fclose(out);

    switch (ret)
    {
    case ERR_OK:
        break;
    case ERR_ALLOC:
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return ret;
    case ERR_ZSTD_INIT:
        fprintf(stderr, "Error: Failed to initialize ZSTD seekable compressor.\n");
        return ret;
    case ERR_ZSTD_COMPRESS:
        fprintf(stderr, "Error: Compression failed.\n");
        return ret;
    default:
        fprintf(stderr, "Error: Unknown error occurred (code %d).\n", ret);
        return ret;
    }

    FILE *in_compressed = fopen_orDie(args.output_file, "rb");
    FILE *in_decompressed = fopen_orDie(args.input_file, "rb");
    int print_ret = zstd_seekable_print_info(in_compressed, in_decompressed);
    if (print_ret)
    {
        fprintf(stderr, "Error: Failed to print seek table info.\n");
        return print_ret;
    }

    return 0;
}
