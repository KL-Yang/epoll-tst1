#include "rfs_s.h"

svr_ctx_t * svr_ctx_init(const char *host, int port, int nthread)
{
    struct epoll_event svr_ev;
    svr_ctx_t *svr = calloc(1, sizeof(svr_ctx_t));

    pthread_spin_init(&svr->lock, PTHREAD_PROCESS_PRIVATE);
    svr->cmd_pool = g_thread_pool_new(server_dispatch, svr, nthread, TRUE, NULL);
    svr->event = calloc (SVR_MAX_EVENT, sizeof(struct epoll_event));
    
    //create listening socket
    svr->socket = svr_socket(host, port, SVR_SERVER | SVR_NONBLOCK);

    //create epoll
    if((svr->efd = epoll_create (SVR_MAX_EVENT)) == -1) {
        perror ("epoll_create");
        abort ();
    }
    fprintf(stderr, "%s: epoll fd=%d svr@%p\n", __func__, svr->efd, svr);

     //add listening event
    svr_ev.data.ptr = svr;
    svr_ev.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(svr->efd, EPOLL_CTL_ADD, svr->socket, &svr_ev) == -1) {
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
