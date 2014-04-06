#include "rfs_s.h"

server_ctx_t * rfss_new_context()
{
    server_ctx_t *ctx;
    ctx = calloc(1, sizeof(server_ctx_t));

    //prepare receiving context
    ctx->cmd = calloc(1, sizeof(rfs_cmd_t));
    ctx->cmd_que = g_queue_new();


    return ctx;
}
