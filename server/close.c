#include "rfs_s.h"

lfd_ctx_t * svr_rfs_close(rfs_close_in_t *in, rfs_close_ou_t *ou)
{
    lfd_ctx_t *lfd = (lfd_ctx_t*)in->ctx;
    while((ou->ret=close(lfd->fd))==-1 && errno==EINTR)
      continue;
    if(ou->ret!=0) ou->rerrno = errno;
    return lfd;
}
