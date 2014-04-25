#include "rfs_s.h"

int svr_monitor_add(int efd, int fd, int flag, void *ptr)
{
    int r;
    struct epoll_event ev;
    ev.data.ptr = ptr;
    ev.events = EPOLLIN | EPOLLET;
    r = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
    if(r==-1) {
        perror("%s: epoll_ctl() failed!\n");
        return 1;
    }
    return 0;
}

int svr_monitor_mod()
{
}

int svr_monitor_del()
{
}
