#include "rfs_s.h"

void svr_run_loop(svr_ctx_t *svr)
{
    int i, n;
    struct epoll_event *evs = svr->event;

    while (1) {
        n = epoll_wait(svr->efd, evs, SVR_MAX_EVENT, -1);
        fprintf(stderr, "%s: epoll_wait n=%d\n", __func__, n);
        for (i = 0; i < n; i++) {
            fprintf(stderr, "%s: ev[%d] data@%p\n", __func__, i, evs[i].data.ptr);

            if ((evs[i].events & EPOLLERR) || (evs[i].events & EPOLLHUP)) {
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
}
