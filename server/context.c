#include "rfs_s.h"

svr_ctx_t * svr_ctx_init(const char *host, int port, int nthread)
{
    svr_ctx_t *svr = calloc(1, sizeof(svr_ctx_t));

    pthread_spin_init(&svr->lock, PTHREAD_PROCESS_PRIVATE);
    svr->cmd_pool = g_thread_pool_new(server_dispatch, svr, nthread, TRUE, NULL);
    svr->event = calloc (SVR_MAX_EVENT, sizeof(struct epoll_event));
    
    //create listening socket
    svr->socket = rfs_sock_connect(host, port, RFS_SOCK_SERVER | RFS_SOCK_NONBLOCK);

    //create epoll
    if((svr->efd = epoll_create (SVR_MAX_EVENT)) == -1) {
        perror ("epoll_create");
        abort ();
    }
    fprintf(stderr, "%s: epoll fd=%d svr@%p\n", __func__, svr->efd, svr);

    int r;
    struct epoll_event svr_ev;

    svr_ev.data.ptr = svr;
    svr_ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    r = epoll_ctl(svr->efd, EPOLL_CTL_ADD, svr->socket, &svr_ev);
    if(r == -1) {
        perror ("epoll_ctl add listen failed!");
        abort ();
    }
    return svr;
}

void svr_ctx_free(svr_ctx_t *svr)
{
    close(svr->efd);
    close(svr->socket);
    g_thread_pool_free(svr->cmd_pool, FALSE, TRUE);
    pthread_spin_destroy(&svr->lock);
    free(svr->event);
    free(svr);
}
