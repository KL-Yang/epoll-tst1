#include "rfs_s.h"

void svr_run_loop(svr_ctx_t *svr)
{
    int i, n;
    struct epoll_event *evs = svr->event;

    while (1) {

        n = epoll_wait(svr->efd, evs, SVR_MAX_EVENT, -1);

        for (i = 0; i < n; i++) {
            if(svr == evs[i].data.ptr) {
                if(evs[i].events & EPOLLIN)
                  svr_accept(svr);
                else {
                    fprintf (stderr, "epoll error\n");
                    abort();
                }
            } 
            else if(evs[i].events & EPOLLIN) 
              svr_inbound(evs[i].data.ptr);
            else if(evs[i].events & EPOLLOUT)
              svr_outbound(evs[i].data.ptr);
            else if(evs[i].events & EPOLLRDHUP) {
                fprintf (stderr, "epoll error EPOLLRDHUP\n");
                abort();
            }
            else  {
                fprintf (stderr, "epoll error\n");
                abort();
            }
        }
    }
}
