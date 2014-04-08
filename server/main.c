#include "rfs_s.h"

uv_loop_t *loop;

static void 
buff_alloc(uv_handle_t* handle, size_t __attribute__((unused)) suggested_size, 
           uv_buf_t* buf) 
{
    rfs_cmd_t *cmd = ((svr_ctx_t*)handle->data)->cmd;
    fprintf(stderr, "%s: handle@%p\n", __func__, handle);
    if(cmd->flag & PCMD_MODE_DATA) {
        buf->len = cmd->head.size - cmd->offset;
        buf->base = (void*)cmd->data + cmd->offset;
    } else {
        buf->len = sizeof(phead_t) - cmd->offset;
        buf->base = (void*)&cmd->head + cmd->offset;
    }
}

static void 
after_read(uv_stream_t* handle, ssize_t nread, 
           const __attribute__((unused)) uv_buf_t* buf)
{
    svr_ctx_t *ctx = handle->data; 
    rfs_cmd_t *cmd = ctx->cmd;
    fprintf(stderr, "%s: handle@%p nread=%ld\n", __func__, handle, nread);
    if(cmd->flag & PCMD_MODE_DATA) {
        if((cmd->offset+=nread)==cmd->head.size) {
            cmd->flag = 0;
            cmd->offset = 0;
            //svr_validate_command();
            g_thread_pool_push(ctx->cmd_pool, cmd, NULL);
            ctx->cmd = calloc(1, sizeof(phead_t));
        }
    } else {
        if((cmd->offset+=nread)==sizeof(phead_t)) {
            cmd->flag |= PCMD_MODE_DATA;
            cmd->data  = malloc(cmd->head.size);
            cmd->offset = 0;
        }
    }
}

void on_new_connection(uv_stream_t *server, int status) 
{
    uv_tcp_t *client;
    if (status == -1) {
        // error!
        return;
    }
    client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    client->data = rfss_new_context(loop);

    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        fprintf(stderr, "%s: server@%p client@%p\n", __func__, server, client);
        uv_read_start((uv_stream_t*) client, buff_alloc, after_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main() {

    loop = uv_loop_new();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    fprintf(stderr, "%s: server@%p\n", __func__, &server);
    struct sockaddr_in bind_addr;
    uv_ip4_addr("0.0.0.0", RFS_PORT, &bind_addr);
    uv_tcp_bind(&server, (const struct sockaddr *)&bind_addr, 0);
    int r = uv_listen((uv_stream_t*) &server, 128, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
