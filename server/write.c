#include "rfs_s.h"

void svr_rfs_write(rfs_write_in_t *in, void **ppou)
{
    lfd_ctx_t *lfd = (lfd_ctx_t*)in->ctx;
    rfs_write_ou_t *ou = malloc(sizeof(rfs_write_ou_t));
    ou->ret = write(lfd->fd, in->buf, in->count);
    ou->rerrno = 0; if(ou->ret!=0) ou->rerrno = errno; 
    *ppou = ou;
}
