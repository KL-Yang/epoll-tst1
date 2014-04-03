#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
uv_loop_t *loop;

static void 
buff_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) 
{
    fprintf(stderr, "%s: handle@%p\n", __func__, handle);
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

static void 
after_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
    fprintf(stderr, "%s: handle@%p\n", __func__, handle);
    int i;
    char *pstr = buf->base;
    for(i=0; i<nread; i++)
      fputc(pstr[i], stderr);
    fputc('\n', stderr);
}

void on_new_connection(uv_stream_t *server, int status) 
{
    if (status == -1) {
        // error!
        return;
    }
    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
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
    uv_ip4_addr("0.0.0.0", 7000, &bind_addr);
    uv_tcp_bind(&server, (const struct sockaddr *)&bind_addr, 0);
    int r = uv_listen((uv_stream_t*) &server, 128, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
