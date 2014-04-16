#include "rfs_s.h"

/**
 * I am the only one who access cln->ret
 * */
int svr_outbound(cln_ctx_t *cln)
{
    fprintf(stderr, "%s: cln@%p\n", __func__, cln);

    int r, done = 0;
    svr_ctx_t *svr = cln->svr;
    void *buf; size_t num; ssize_t count;

    while(!done) {

        if(cln->ret==NULL) {
            pthread_spin_lock(&cln->lock);
            if(cln->rqd>0) {
                cln->ret = g_queue_pop_head(cln->ret_que);
                cln->rqd--;
                fprintf(stderr, "%s: pop up ret@%p rqd=%d\n", __func__, 
                        cln->ret, cln->rqd);
            } else done = 1; 
            pthread_spin_unlock(&cln->lock);
        }

        if(done) break;

        rfs_cmd_t *cmd = cln->ret;

        if(cmd->flag & PCMD_MODE_DATA) {
            buf = cmd->data + cmd->offset;
            num = cmd->head.size - cmd->offset;
        } else {
            buf = (void*)&cmd->head + cmd->offset;
            num = sizeof(phead_t) - cmd->offset;
        }
        fprintf(stderr, "%s: write num=%d\n", __func__, (int)num);
        count = write(cln->socket, buf, num);
        if(count == -1) {
            if(errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("read error code not EAGAIN");
                abort();
            }
            break;
        } else if (count == 0) {
            /* broke or closed socket, handle it here! */
            break;
        }
        cmd->offset += count;
        if(cmd->flag & PCMD_MODE_DATA) {
            if(cmd->offset == cmd->head.size) {
                cln->ret = NULL;
                free(cmd->data);
                free(cmd);
            }
        } else {
            if(cmd->offset == sizeof(phead_t)) {
                cmd->flag |= PCMD_MODE_DATA;
                cmd->offset = 0;
            }
        }
    }

    struct epoll_event event;
    if(done) {
        pthread_spin_lock(&cln->lock);
        event.data.ptr = cln;   //We better record the event, we only disable EPOLLOUT!
        event.events = EPOLLIN; //do not touch other setting.
        if((r=epoll_ctl(svr->efd, EPOLL_CTL_MOD, cln->socket, &event)) == -1) {
            perror("epoll_ctl ");
            abort();
        }
        pthread_spin_unlock(&cln->lock);
    }

    fprintf(stderr, "%s: finish!\n", __func__);
    return 0;
}
