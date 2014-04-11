#include "rfs_s.h"

cln_ctx_t * svr_new_client(svr_ctx_t *svr, uv_tcp_t *client)
{
    cln_ctx_t *cln = calloc(1, sizeof(cln_ctx_t));
    cln->svr = svr;
    cln->cmd = calloc(1, sizeof(rfs_cmd_t));
    cln->cmd->ctx = cln;
    cln->client = (uv_stream_t*)client;
    cln->ret_que = g_queue_new();
    return cln;
}
