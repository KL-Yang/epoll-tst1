#include "rfs_s.h"

void svr_rfs_write(rfs_write_in_t *in, rfs_write_ou_t *ou)
{
    lfd_ctx_t *lfd = (lfd_ctx_t*)in->ctx;
    ou->ret = write(lfd->fd, in->buf, in->count);
    if(ou->ret!=0) ou->rerrno = errno;
}
