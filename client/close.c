#include "rfs_c.h"

int rfs_close(int64_t rfs_handle)
{
    int ret, socket;
    rfs_t *rfs = (rfs_t*)rfs_handle;
    socket = rfs->socket;

    phead_t *h = rfs_protocol_new_head(RFS_CLOSE, rfs);
    h->size = sizeof(rfs_close_in_t);

    rfs_close_in_t *idata = calloc(h->size, 1);
    idata->ctx = rfs->ctx;

    if(write(socket, h, sizeof(phead_t))!=sizeof(phead_t))
      ABORT_ME("%s: Fail to send protocol header\n", __func__);

    if(write(socket, idata, h->size)!=h->size)
      ABORT_ME("%s: Fail to send payload\n", __func__);
    free(idata);

    //get information of closed file
    if(read(socket, h, sizeof(phead_t))!=sizeof(phead_t))
      ABORT_ME("%s: Fail to recv protocol header\n", __func__);

    rfs_close_ou_t *odata = calloc(h->size, 1);
    if(read(socket, odata, h->size)!=h->size)
      ABORT_ME("%s: Fail to recv payload\n", __func__);

    ret = odata->ret;
    errno = odata->rerrno;
    free(odata);
    free(h);
    return ret;
}
