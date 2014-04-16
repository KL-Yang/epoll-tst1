#include "rfs_s.h"

lfd_ctx_t * svr_rfs_open(rfs_open_in_t *in, void **ppou)
{
    lfd_ctx_t *ctx = calloc(1, sizeof(lfd_ctx_t));
    rfs_open_ou_t *ou = malloc(sizeof(rfs_open_ou_t));
    *ppou = ou;
    if((ctx->fd=open(in->pathname, in->flags))==-1) {
        ou->ctx = 0;
        ou->rerrno = errno;
        free(ctx);
        perror("Error");
        abort();
        return NULL;
    }
    ctx->magic = random();
    ctx->sequence = 0;
    ou->ctx = (int64_t)ctx;
    fprintf(stderr, "%s: ou->ctx=%ld\n", __func__, ou->ctx);
    ou->rerrno = 0;
    return ctx;
}
