#include "rfs_s.h"

cln_ctx_t * svr_new_client(int socket, svr_ctx_t *svr)
{
    cln_ctx_t *cln = calloc(1, sizeof(cln_ctx_t));
    cln->svr = svr;
    cln->socket = socket;
    cln->cmd = calloc(1, sizeof(rfs_cmd_t));
    cln->cmd->ctx = cln;
    cln->ret_que = g_queue_new();
    pthread_spin_init(&cln->lock, PTHREAD_PROCESS_PRIVATE);
    return cln;
}

int svr_accept(svr_ctx_t *svr)
{
    int r, fd;
    socklen_t len;
    struct sockaddr addr;
    struct epoll_event event;

    while (1) {
        len = sizeof(addr);
        if((fd = accept (svr->socket, &addr, &len)) == -1) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                break;  //has processed all incoming connection
            else {
                fprintf(stderr, "%s: accept() failed %s\n", __func__, strerror(errno));
                abort();
            }
        }
        fprintf(stderr, "%s: sfd=%d, fd=%d\n", __func__, svr->socket, fd);

        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        getnameinfo (&addr, len, hbuf, sizeof hbuf, sbuf, sizeof sbuf,
                     NI_NUMERICHOST | NI_NUMERICSERV);
        fprintf(stderr, "Accepted connection on descriptor %d (host=%s, port=%s)\n", 
               fd, hbuf, sbuf);

        /* Make the incoming socket non-blocking and add it to the
           list of fds to monitor. */
        if((r = svr_make_non_block(fd)) == -1)
          abort ();

        cln_ctx_t *cln = svr_new_client(fd, svr);
        svr->cln_list = g_list_append(svr->cln_list, cln);
        fprintf(stderr, "%s: new cln@%p\n", __func__, cln);

        event.data.ptr = cln;
        event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
        r = epoll_ctl(svr->efd, EPOLL_CTL_ADD, fd, &event);
        if(r == -1) {
            fprintf(stderr, "%s: epoll_ctl failed %s\n", __func__, strerror(errno));
            abort ();
        }
    }
    return fd;
}
