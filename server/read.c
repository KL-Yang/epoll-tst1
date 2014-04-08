#include "rfs_s.h"

void svr_rfs_read(rfs_read_in_t *in, void *ppou)
{
    lfd_ctx_t *lfd = (lfd_ctx_t*)in->ctx;
    rfs_read_ou_t *ou = malloc(sizeof(rfs_read_ou_t)+in->count);
    ou->ret = read(lfd->fd, ou->buf, in->count);
    if(ou->ret!=0) ou->rerrno = errno; else ou->rerrno = 0;
    *((rfs_read_ou_t**)ppou) = ou;
}
