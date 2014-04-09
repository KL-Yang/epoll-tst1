#include "rfs_s.h"

static void 
trigger_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) 
{
    size_t alloc_size = suggested_size - suggested_size%sizeof(cln_ctx_t*);
    buf->len = alloc_size;
    buf->base = malloc(alloc_size);
    fprintf(stderr, "%s: base@%p len=%5d\n", __func__, buf->base, (int)buf->len);
}

static void
after_trigger(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
    if(nread%sizeof(cln_ctx_t*)!=0) 
      ABORT_ME("%s: It's wired to get an incomplete pointer!\n", __func__);

    int i, num;
    cln_ctx_t **cln;
    num = nread/sizeof(cln_ctx_t*);
    cln = (cln_ctx_t**)buf->base;
    for(i=0; i<num; i++) {
        //pop a return command from cln[i]->ret_que
        //uv_write();
    }
    free(buf->base);
}

void beacon_connection(uv_stream_t *server, int status) 
{
    if (status == -1) {
        /* error! */ return;
    }

    svr_ctx_t *svr;
    uv_tcp_t *trigger;
    trigger = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    trigger->data = svr = server->data;
    svr->trigger = trigger;

    uv_tcp_init(svr->loop, trigger);
    if (uv_accept(server, (uv_stream_t*) trigger) == 0) {
        fprintf(stderr, "%s: server@%p trigger@%p\n", __func__, server, trigger);
        uv_read_start((uv_stream_t*) trigger, trigger_alloc, after_trigger);
    }
    else {
        uv_close((uv_handle_t*) trigger, NULL);
    }
}

