#include "rfs_s.h"

lfd_ctx_t * svr_rfs_close(rfs_close_in_t *in, void **ppou)
{
    lfd_ctx_t *lfd = (lfd_ctx_t*)in->ctx;
    rfs_close_ou_t *ou = malloc(sizeof(rfs_close_ou_t));
    //while((ou->ret=close(lfd->fd))==-1 && errno==EINTR)
    //  continue;
    ou->ret = 0;
    ou->rerrno = 0; if(ou->ret!=0) ou->rerrno = errno; 
    *ppou = ou;
    return lfd;
}
