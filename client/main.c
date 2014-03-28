#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
uv_loop_t *loop;

static void 
on_connect(uv_connect_t *client, int status) 
{
    fprintf(stderr, "%s: client@%p\n", __func__, client);
    /*
    if (status == -1) {
        // error!
        return;
    }
    return;
    */
}

int main()
{
    int r;
    loop = uv_loop_new();

    uv_tcp_t client;
    uv_tcp_init(loop, &client);
    /*
    uv_os_sock_t sock;
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    int r = uv_tcp_open(&client, sock);
    */
    uv_connect_t connect;

    fprintf(stderr, "%s: client@%p connect@%p\n", __func__, &client, &connect);
    struct sockaddr_in dest;
    uv_ip4_addr("127.0.0.1", 7000, &dest);

    r = uv_tcp_connect(&connect, &client, (const struct sockaddr*)&dest, on_connect);

    return uv_run(loop, UV_RUN_DEFAULT);
}
