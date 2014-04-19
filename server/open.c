#include "rfs_s.h"

lfd_ctx_t * 
svr_rfs_open(rfs_open_in_t *in, void **ppou)
{
    int fd;
    rfs_open_ou_t *ou; 
    ou = *ppou = malloc(sizeof(rfs_open_ou_t));

    fd = open(in->pathname, in->flags);
    if(fd == -1) {
        ou->ctx = 0;
        ou->rerrno = errno;
        return NULL;
    } 

    lfd_ctx_t *ctx;
    ctx  = calloc(1, sizeof(lfd_ctx_t));
    ctx->fd = fd;
    ctx->magic = random();
    ctx->sequence = 0;

    ou->ctx = (int64_t)ctx;
    ou->rerrno = 0;
    return ctx;
}
