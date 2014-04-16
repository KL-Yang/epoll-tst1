#include "rfs_s.h"

int svr_inbound(cln_ctx_t *cln)
{
    svr_ctx_t *svr = cln->svr;
    rfs_cmd_t *cmd = cln->cmd;
    void *buf; size_t num; ssize_t count;
    fprintf(stderr, "%s\n", __func__);

    while(1) {
        if(cmd->flag & PCMD_MODE_DATA) {
            buf = cmd->data + cmd->offset;
            num = cmd->head.size - cmd->offset;
        } else {
            buf = (void*)&cmd->head + cmd->offset;
            num = sizeof(phead_t) - cmd->offset;
        }
        count = read(cln->socket, buf, num);
        if(count == -1) {
            if(errno != EAGAIN) {
                perror("read error code not EAGAIN");
                abort();
            }
            break;
        } else if (count == 0) {
            /* broke or closed socket, handle it here! */
            perror("socket broken!");
            abort();
            break;
        }
        cmd->offset += count;
        if(cmd->flag & PCMD_MODE_DATA) {
            if(cmd->offset == cmd->head.size) {
                cmd->flag = 0;
                cmd->offset = 0;
                g_thread_pool_push(svr->cmd_pool, cmd, NULL);
                cmd = calloc(1, sizeof(rfs_cmd_t));
                cmd->ctx = cln;
                cln->cmd = cmd;
            }
        } else {
            if(cmd->offset == sizeof(phead_t)) {
                cmd->flag |= PCMD_MODE_DATA;
                cmd->data  = malloc(cmd->head.size);
                cmd->offset = 0;
                switch(cmd->head.func_id) {
                  case RFS_OPEN:
                    fprintf(stderr, "%s:   RFS_OPEN()\n", __func__);
                    break;
                  case RFS_CLOSE:
                    fprintf(stderr, "%s:   RFS_OPEN()\n", __func__);
                    break;
                  default:
                    abort();
                }
            }
        }
    }
    return 0;
}
