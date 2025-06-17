#include <argp.h>
#include <stdlib.h>
#include <stdio.h>

#include "zstd.h"
#include "zstd_seekable.h"
#include "common.h"

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

    ZSTD_seekable_initCStream(zcs, compression_level, 1, frame_size);
    size_t read, to_read = buff_in_size;
    while( (read = fread_orDie(buff_in, to_read, in)) )
    {
        ZSTD_inBuffer input = {buff_in, read, 0};
        ZSTD_outBuffer output = { buff_out, buff_out_size, 0 };
        to_read = ZSTD_seekable_compressStream(zcs, &output, &input);
        if (to_read > buff_in_size) to_read = buff_in_size;
        fwrite_orDie(buff_out, output.pos, out);
    }

    while(1) {
        ZSTD_outBuffer output = { buff_out, buff_out_size, 0 };
        size_t const remaining_to_flush = ZSTD_seekable_endStream(zcs, &output);
        CHECK_ZSTD(remaining_to_flush);
        fwrite_orDie(buff_out, output.pos, out);
        if (!remaining_to_flush) break;
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

    if (args.frame_size == 0) {
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

    return 0;
}
