#include "rfs_s.h"

svr_ctx_t * svr_ctx_new()
{
    svr_ctx_t *svr;
    svr = calloc(1, sizeof(svr_ctx_t));
    pthread_spin_init(&svr->lock, PTHREAD_PROCESS_PRIVATE);
    svr->cmd_pool = g_thread_pool_new(server_dispatch, svr, 2, TRUE, NULL);
    svr->event = calloc (SVR_MAX_EVENT, sizeof(struct epoll_event));
    return svr;
}

void svr_ctx_free(svr_ctx_t *svr)
{
    pthread_spin_destroy(&svr->lock);
    g_thread_pool_free(svr->cmd_pool, FALSE, TRUE);
    free(svr);
}
