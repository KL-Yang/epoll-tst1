#include "rfs_s.h"

void * svr_notify_try_connect(void *arg)
{
    svr_ctx_t *svr = arg;
    fprintf(stderr, "%s: waiting for semaphore\n", __func__);
    //while(sem_wait(&svr->notify_sem)!=0 && errno==EINTR)
    //  continue;
    svr->ping = rfs_socket_connect("localhost", RFS_BEACON_PORT);
    fprintf(stderr, "%s: svr->ping=%d\n", __func__, svr->ping);
    return NULL;
}


static void
after_notify_write(uv_write_t* req, int status) 
{
    fprintf(stderr, "%s: status=%d\n", __func__, status);
    uv_buf_t *buf = ((rfs_cmd_t*)(req->data))->data;
    free(buf[1].base);  //original cmd->data
    free(req->data);    //original cmd
    free(buf);
    free(req);
}

void
after_notify(uv_stream_t __attribute__ ((unused)) *handle, ssize_t nread, 
             const uv_buf_t* buf)
{
    if(nread%sizeof(cln_ctx_t*)!=0) 
      ABORT_ME("%s: It's wired to get an incomplete pointer!\n", __func__);

    int i, num = nread/sizeof(cln_ctx_t*);
    cln_ctx_t **cln = (cln_ctx_t**)buf->base;

    for(i=0; i<num; i++) 
    {
        fprintf(stderr, "%s: client[%d]@%p\n", __func__, i, cln[i]);
        pthread_spin_lock(&cln[i]->lock);
        rfs_cmd_t *cmd = g_queue_pop_head(cln[i]->ret_que);
        cln[i]->rqd--;
        pthread_spin_unlock(&cln[i]->lock);

        uv_buf_t *ret = calloc(2, sizeof(uv_buf_t));
        uv_write_t *req = calloc(1, sizeof(uv_write_t));
        ret[0].len = sizeof(phead_t);
        ret[1].len = cmd->head.size;
        ret[0].base = (void*)&cmd->head;
        ret[1].base = cmd->data;
        cmd->data = ret; 
        req->data = cmd;

        uv_write(req, cln[i]->client, ret, 2, after_notify_write);
    }
    free(buf->base);    //by notify_alloc;
}

void svr_notify_setup(uv_stream_t *server, int status) 
{
    if (status == -1) {
        fprintf(stderr, "%s: error status=%d\n", __func__, status);
        /* error! */ return;
    }
    int r;
    svr_ctx_t *svr = server->data;
    svr->notify = (uv_stream_t*) calloc(1, sizeof(uv_tcp_t));
    uv_tcp_init(svr->loop, (uv_tcp_t*)svr->notify);
    r = uv_accept(server, (uv_stream_t*)svr->notify);
    ABORT_ON(r, "%s: failed to establish notify connection!\n", __func__);
    uv_close((uv_handle_t*)server, NULL);
}
