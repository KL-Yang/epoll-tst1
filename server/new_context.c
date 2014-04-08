#include "rfs_s.h"

svr_ctx_t * rfss_new_context()
{
    svr_ctx_t *ctx;
    ctx = calloc(1, sizeof(svr_ctx_t));

    pthread_spin_init(&ctx->lock, PTHREAD_PROCESS_PRIVATE);

    //prepare receiving context
    ctx->cmd = calloc(1, sizeof(rfs_cmd_t));
    ctx->cmd_pool = g_thread_pool_new(server_dispatch, ctx, 2, TRUE, NULL);

    return ctx;
}
