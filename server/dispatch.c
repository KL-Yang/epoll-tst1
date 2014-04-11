#include "rfs_s.h"

void server_dispatch(void *data, void *user_data)
{
    void *dout; 
    lfd_ctx_t *lfd;
    svr_ctx_t *svr = (svr_ctx_t*)user_data;
    rfs_cmd_t *cmd = (rfs_cmd_t*)data;
    cln_ctx_t *cln = cmd->ctx;
    fprintf(stderr, "%s: cmd@%p\n", __func__, cmd);

    switch(cmd->head.func_id) {
      case RFS_OPEN:
        fprintf(stderr, "%s: RFS_OPEN\n", __func__);
        lfd = svr_rfs_open(cmd->data, &dout);
        pthread_spin_lock(&svr->lock);
        svr->lfd_list = g_list_append(svr->lfd_list, lfd);
        pthread_spin_unlock(&svr->lock);
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
        pthread_spin_lock(&svr->lock);
        svr->lfd_list = g_list_remove(svr->lfd_list, lfd);
        pthread_spin_unlock(&svr->lock);
        break;

      case RFS_STAT:
        break;

      case RFS_SEEK:

      default:
        ABORT_ME("Unknow function id %d\n", cmd->head.func_id);
    }
    free(cmd->data);
    cmd->data = dout;

    g_queue_push_tail(cln->ret_que, cmd);
    cln->rqd++;

    //ping the notify socket!
    fprintf(stderr, "%s: ping the socket[%d]!\n", __func__, svr->ping);
    if(write(svr->ping, &cln, sizeof(void*))!=sizeof(void*)) 
      ABORT_ME("Failed to notify a command has completed!\n");
    fprintf(stderr, "%s: done the ping[%d]!\n", __func__, svr->ping);
}
