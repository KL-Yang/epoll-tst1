#include "rfs_s.h"

static void 
buff_alloc(uv_handle_t* handle, size_t __attribute__((unused)) suggested_size, 
           uv_buf_t* buf) 
{
    rfs_cmd_t *cmd = ((svr_ctx_t*)handle->data)->cmd;
    if(cmd->flag & PCMD_MODE_DATA) {
        buf->len = cmd->head.size - cmd->offset;
        buf->base = (void*)cmd->data + cmd->offset;
    } else {
        buf->len = sizeof(phead_t) - cmd->offset;
        buf->base = (void*)&cmd->head + cmd->offset;
    }
    fprintf(stderr, "%s: base@%p len=%5d\n", __func__, buf->base, (int)buf->len);
}

static void 
after_read(uv_stream_t* handle, ssize_t nread, 
           const __attribute__((unused)) uv_buf_t* buf)
{
    if(nread<0) {
        uv_read_stop((uv_stream_t*)handle);
        return;
    }
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
            fprintf(stderr, "%s: ctx->cmd allocate@%p\n", __func__, ctx->cmd);
        }
    } else {
        if((cmd->offset+=nread)==sizeof(phead_t)) {
            cmd->flag |= PCMD_MODE_DATA;
            cmd->data  = malloc(cmd->head.size);
            cmd->offset = 0;
            fprintf(stderr, "   head->protocol=%4s\n", cmd->head.protocol);
        }
    }
}

static void server_connection(uv_stream_t *server, int status) 
{
    if (status == -1) {
        /* error! */ return;
    }

    svr_ctx_t *svr;
    uv_tcp_t *client;
    svr = server->data;
    client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    //new svr_new_client();
    //client->data = rfss_new_context((uv_stream_t*)client);
    svr->cln_list = g_list_append(svr->cln_list, client);

    uv_tcp_init(svr->loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        fprintf(stderr, "%s: server@%p client@%p\n", __func__, server, client);
        uv_read_start((uv_stream_t*) client, buff_alloc, after_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}



int main() {

    int r;
    svr_ctx_t *svr;
    uv_tcp_t *server, *beacon;
    struct sockaddr_in server_addr, beacon_addr;

    /**
     * setup server context, used by all
     * one connection per context!, 
     * */
    svr = calloc(1, sizeof(svr_ctx_t));
    svr->loop = uv_loop_new();

    /**
     * listen to triger and create a self connection
     * */
    beacon = calloc(1, sizeof(uv_tcp_t));
    beacon->data = svr;
    uv_tcp_init(svr->loop, beacon);
    uv_ip4_addr("127.0.0.1", RFS_BEACON_PORT, &beacon_addr);
    uv_tcp_bind(beacon, (const struct sockaddr *)&beacon_addr, 0);
    if((r=uv_listen((uv_stream_t*) beacon, 128, beacon_connection))!=0) {
        fprintf(stderr, "Beacon listen error %s\n", uv_err_name(r));
        return 1;
    }
    /**
     * connect to the self trigger and initiate context
     * */
    svr->bcon = lib_socket_connect("127.0.0.1", RFS_BEACON_PORT);

    /**
     * other part of server initiation, move rfss_new_context here!
     * */
    pthread_spin_init(&svr->lock, PTHREAD_PROCESS_PRIVATE);
    svr->cmd_pool = g_thread_pool_new(server_dispatch, svr, 2, TRUE, NULL);

    /**
     * listen as rfs server and setup client context
     * */
    server = calloc(1, sizeof(uv_tcp_t));
    server->data = svr;
    uv_tcp_init(svr->loop, server);
    uv_ip4_addr("0.0.0.0", RFS_PORT, &server_addr);
    uv_tcp_bind(server, (const struct sockaddr *)&server_addr, 0);
    if((r=uv_listen((uv_stream_t*) server, 128, server_connection))!=0) {
        fprintf(stderr, "Server listen error %s\n", uv_err_name(r));
        return 1;
    }

    /**
     * finished server context creation, start event loop
     * */
    r = uv_run(svr->loop, UV_RUN_DEFAULT);

    free(svr);
    free(server);
    free(beacon);

    return r;
}
