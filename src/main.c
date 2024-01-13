#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    while (ptr) {
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

int main(int argc, char** argv)
{
    if (argc < 4) {
        fprintf(stderr, "expected arguments\n");
        return 1;
    }

    char* search = argv[2];
    char* replace = argv[3];

    char* file_content = read_whole_file(argv[1]);
    if (!file_content)
        return 1;

    correction_ctx_t ctx = {0};
    correction_init(&ctx, file_content, search, replace);

    char* replaced = replace_strings(&ctx);
    if (!replaced)
        return 1;

    printf("%s", replaced);

    free(replaced);
    free(file_content);
    return 0;
}
