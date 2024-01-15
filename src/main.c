#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char* read_whole_file(const char* filepath)
{
    FILE* file = fopen(filepath, "r");
    if (!file)
        return NULL;

    fseek(file, SEEK_SET, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size == 0)
        return NULL;

    char* content = malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';

    fclose(file);

    return content;
}

typedef struct {
    const char* source;
    const char* matched_strs[2048];
    size_t matched_count;
    const char* search;
    const char* replace_by;
} correction_ctx_t;

void correction_init(correction_ctx_t* ctx, const char* source, const char* search, const char* replace_by)
{
    ctx->source = source;
    ctx->search = search;
    ctx->replace_by = replace_by;
}

void match_strings(correction_ctx_t* ctx)
{
    const char* source = ctx->source;

    const char* ptr = strstr(source, ctx->search);
    size_t count = 0;

    while (ptr && count < 2048) {
        ctx->matched_strs[count++] = ptr;
        source = ptr + strlen(ctx->search);
        ptr = strstr(source, ctx->search);
    }

    ctx->matched_count = count;
}

char* replace_strings(correction_ctx_t* ctx)
{
    match_strings(ctx);

    if (ctx->matched_count == 0)
        return NULL;

    size_t new_size = (strlen(ctx->source) - (strlen(ctx->search) * ctx->matched_count)) + strlen(ctx->replace_by) * ctx->matched_count;
    char* new_source = malloc(sizeof(char) * new_size + 1);
    char* ptr = new_source;

    size_t source_len = strlen(ctx->source);
    for (size_t i = 0; i < source_len; i++) {
        for (size_t j = 0; j < ctx->matched_count; j++) {
            if (ctx->source + i == ctx->matched_strs[j]) {
                size_t replace_by_count = strlen(ctx->replace_by);

                strncpy(ptr, ctx->replace_by, replace_by_count);
                ptr += replace_by_count;
                
                // skip matched string
                i += strlen(ctx->search);
            }
        }

        strncpy(ptr, &ctx->source[i], 1);
        ptr += 1;
    }

    new_source[new_size] = '\0';
    return new_source;
}

void rewind_args(int* argc, char*** argv)
{
    *argc += 1;
    *argv = *argv - 1;
}

char* shift_args(int* argc, char*** argv)
{
    if (argc == 0) {
        return NULL;
    }

    char* current = **argv;
    *argv = *argv + 1;
    *argc -= 1;

    return current;
}

typedef struct {
    const char* search_str;
    const char* replace_str;

    const char* input_file;
    const char* output_file;

    bool forward_output;
    bool forward_stdout;

    bool case_insensitive; // TODO: implement this.
} flags_t;

void usage(const char* program)
{
    fprintf(stderr, "usage: %s <input_file> [flags..] <search_str> <replace_str>\n", program);
    fprintf(stderr, "flags:\n");
    fprintf(stderr, "   -fnostdout              don't forward the output to the stdout.\n");
    fprintf(stderr, "   -foutput <output_file>  forward the output to file\n");
    fprintf(stderr, "   -nocase                 case insensitive search\n");
}
 
flags_t parse_arguments(int argc, char** argv)
{
    flags_t flags = {
        .search_str = NULL,
        .replace_str = NULL,

        .input_file = NULL,
        .output_file = NULL,

        .forward_output = false,
        .forward_stdout = true,

        .case_insensitive = false,
    };

    const char* program = shift_args(&argc, &argv);

    const char* input_file = shift_args(&argc, &argv);
    if (!input_file) {
        usage(program);
        fprintf(stderr, "ERROR: expected input file\n");
        exit(1);
    }

    flags.input_file = input_file;

    const char* argument = shift_args(&argc, &argv);
    while (argument) {
        if (strcmp(argument, "-fnostdout") == 0) {
            flags.forward_stdout = false;
        } else if (strcmp(argument, "-foutput") == 0) {
            const char* output_file = shift_args(&argc, &argv);

            if (!output_file) {
                usage(program);
                fprintf(stderr, "ERROR: expected output file\n");
                exit(1);
            }

            flags.output_file = output_file;
            flags.forward_output = true;
        } else if (strcmp(argument, "-nocase") == 0) {
            flags.case_insensitive = true;
        } else {
            // if your search string is one of this flags, then it will cause a problem...
            rewind_args(&argc, &argv);
            break;
        }

        argument = shift_args(&argc, &argv);
    }

    const char* search_str = shift_args(&argc, &argv);
    if (!search_str) {
        usage(program);
        fprintf(stderr, "ERROR: expected search string\n");
        exit(1);
    }

    const char* replace_str = shift_args(&argc, &argv);
    if (!replace_str) {
        usage(program);
        fprintf(stderr, "ERROR: expected replace string\n");
        exit(1);
    }

    flags.search_str = search_str;
    flags.replace_str = replace_str;

    return flags;
}

int main(int argc, char** argv)
{
    flags_t flags = parse_arguments(argc, argv);

    char* file_content = read_whole_file(flags.input_file);
    if (!file_content)
        return 1;

    correction_ctx_t ctx = {0};
    correction_init(&ctx, file_content, flags.search_str, flags.replace_str);

    char* replaced = replace_strings(&ctx);

    if (flags.forward_stdout) {
        if (!replaced) {
            printf("%s", file_content);
        } else {
            printf("%s", replaced);
        }
    }

    if (flags.forward_output) {
        FILE* output = fopen(flags.output_file, "w");
        if (!output) {
            fprintf(stderr, "ERROR: cannot write to file\n");
            return 1;
        }

        if (!replaced) {
            fwrite(file_content, sizeof(char), strlen(file_content), output);
        } else {
            fwrite(replaced, sizeof(char), strlen(replaced), output);
        }
    }

    free(replaced);
    free(file_content);
    return 0;
}
