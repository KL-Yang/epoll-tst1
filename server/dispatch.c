#include "rfs_s.h"

void server_dispatch(void *data, void *user_data)
{
    void *ret; 
    lfd_ctx_t *lfd;
    rfs_cmd_t *cmd = (rfs_cmd_t*)data;
    svr_ctx_t *ctx = (svr_ctx_t*)user_data;

    switch(cmd->head.func_id) {
      case RFS_OPEN:
        fprintf(stderr, "%s: RFS_OPEN\n", __func__);
        ret = calloc(1, sizeof(rfs_open_ou_t));
        lfd = svr_rfs_open(cmd->data, ret);
        pthread_spin_lock(&ctx->lock);
        ctx->lfd_list = g_list_append(ctx->lfd_list, lfd);
        pthread_spin_unlock(&ctx->lock);
        break;

      case RFS_READ:
        fprintf(stderr, "%s: RFS_READ\n", __func__);
        svr_rfs_read(cmd->data, &ret);
        break;

      case RFS_WRITE:
        fprintf(stderr, "%s: RFS_WRITE\n", __func__);
        ret = calloc(1, sizeof(rfs_write_ou_t));
        svr_rfs_write(cmd->data, ret);
        break;

      case RFS_CLOSE:
        fprintf(stderr, "%s: RFS_CLOSE\n", __func__);
        ret = calloc(1, sizeof(rfs_close_ou_t));
        lfd = svr_rfs_close(cmd->data, ret);
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
    cmd->data = ret;
    pthread_spin_lock(&ctx->lock);
    g_queue_push_tail(ctx->ret_que, cmd);
    ctx->rqd++;
    pthread_spin_unlock(&ctx->lock);
}
