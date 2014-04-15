#include "rfs_s.h"

int svr_outbound(cln_ctx_t *cln)
{
    fprintf(stderr, "%s: cln@%p\n", __func__, cln);
    return 0;

    svr_ctx_t *svr = cln->svr;
    rfs_cmd_t *cmd = cln->cmd;
    void *buf; size_t num; ssize_t count;

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
        } else if (num == 0) {
            /* broke or closed socket, handle it here! */
            break;
        }
        cmd->offset += num;
        if(cmd->flag & PCMD_MODE_DATA) {
            if(cmd->offset == cmd->head.size) {
                cmd->flag = 0;
                cmd->offset = 0;
                g_thread_pool_push(svr->cmd_pool, cmd, NULL);
                cmd = calloc(1, sizeof(phead_t));
                cmd->ctx = cln;
                cln->cmd = cmd;
            }
        } else {
            if(cmd->offset == sizeof(phead_t)) {
                cmd->flag |= PCMD_MODE_DATA;
                cmd->data  = malloc(cmd->head.size);
                cmd->offset = 0;
                fprintf(stderr, "%s:   head->protocol=%4s\n", __func__, cmd->head.protocol);
            }
        }
    }
    return 0;
}
