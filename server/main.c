#include "rfs_s.h"

int main() {

    svr_ctx_t *svr;
    int i, n, sfd, efd;
    struct epoll_event *evs, svr_ev;

    svr = svr_ctx_new();
    /**
     * Create one hanle and one event only, used to setup the self
     * notificition system. Abort if cannot establish the connection,
     * we cannot continue without it.
     * */
    sfd = svr_socket("127.0.0.1", RFS_PORT, SVR_SERVER | SVR_NONBLOCK);

    if((efd = epoll_create (SVR_MAX_EVENT)) == -1) {
        perror ("epoll_create");
        abort ();
    }
    fprintf(stderr, "%s: epoll fd=%d svr@%p\n", __func__, efd, svr);

    svr->efd = efd;
    svr->socket = sfd;

    /**
     * add listening event
     * */
    svr_ev.data.ptr = svr;
    svr_ev.events = EPOLLIN | EPOLLET;
    if(epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &svr_ev) == -1) {
        perror ("epoll_ctl add listen failed!");
        abort ();
    }

    /* The event loop */
    evs = svr->event;
    while (1) {
        n = epoll_wait(efd, evs, SVR_MAX_EVENT, -1);
        fprintf(stderr, "%s: epoll_wait n=%d\n", __func__, n);
        for (i = 0; i < n; i++) {
            fprintf(stderr, "%s: ev[%d] data@%p\n", __func__, i, evs[i].data.ptr);

            if ((evs[i].events & EPOLLERR) || (evs[i].events & EPOLLHUP))
            {
                /* An error has occured on this fd, or the socket is not
                   ready for reading (why were we notified then?) */
                fprintf (stderr, "epoll error\n");
                //close (events[i].data.fd);
                continue;
            } else if (svr == evs[i].data.ptr) {
                svr_accept(svr);
            } else {
                if(evs[i].events & EPOLLIN) 
                  svr_inbound(evs[i].data.ptr);
                if(evs[i].events & EPOLLOUT)
                  svr_outbound(evs[i].data.ptr);
            }
        }
    }

    svr_ctx_free(svr);
    close(sfd);
    close(efd);

    return 0;
}
