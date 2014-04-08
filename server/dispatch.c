#include "rfs_s.h"

static void after_write(uv_write_t* req, int status) {
    ABORT_ON(req != NULL, "NULL handle!");
    ABORT_ON(status, "uv_write error: %s\n", uv_strerror(status));
    rfs_ret_t *ret = (rfs_ret_t*)req;
    free(ret->cmd->data);
    free(ret->cmd);
    free(ret);
}


void server_dispatch(void *data, void *user_data)
{
    void *dout; 
    lfd_ctx_t *lfd;
    rfs_cmd_t *cmd = (rfs_cmd_t*)data;
    svr_ctx_t *ctx = (svr_ctx_t*)user_data;

    switch(cmd->head.func_id) {
      case RFS_OPEN:
        fprintf(stderr, "%s: RFS_OPEN\n", __func__);
        lfd = svr_rfs_open(cmd->data, &dout);
        pthread_spin_lock(&ctx->lock);
        ctx->lfd_list = g_list_append(ctx->lfd_list, lfd);
        pthread_spin_unlock(&ctx->lock);
        break;

      case RFS_READ:
        fprintf(stderr, "%s: RFS_READ\n", __func__);
        svr_rfs_read(cmd->data, &dout);
        break;

      case RFS_WRITE:
        fprintf(stderr, "%s: RFS_WRITE\n", __func__);
        svr_rfs_write(cmd->data, &dout);
        break;

      case RFS_CLOSE:
        fprintf(stderr, "%s: RFS_CLOSE\n", __func__);
        lfd = svr_rfs_close(cmd->data, &dout);
        pthread_spin_lock(&ctx->lock);
        ctx->lfd_list = g_list_remove(ctx->lfd_list, lfd);
        pthread_spin_unlock(&ctx->lock);
        break;

      case RFS_STAT:
        break;

      case RFS_SEEK:

      default:
        ABORT_ME("Unknow function id %d\n", cmd->head.func_id);
    }
    free(cmd->data);
    cmd->data = dout;

    rfs_ret_t *ret = calloc(1, sizeof(rfs_ret_t));
    ret->cmd = cmd;
    ret->buf[0].base = (void*)&cmd->head;
    ret->buf[0].len = sizeof(phead_t);
    ret->buf[1].base = cmd->data;
    ret->buf[1].len = cmd->head.size;

    pthread_spin_lock(&ctx->lock);
    uv_write(&ret->req, ctx->client, ret->buf, 2, after_write);
    g_queue_push_tail(ctx->ret_que, cmd);
    ctx->rqd++;
    pthread_spin_unlock(&ctx->lock);
}
