#include "rfs_c.h"

int64_t rfs_read(int64_t rfd, void *buf, int64_t count)
{
    int ret, socket;
    rfs_t *rfs = (rfs_t*)rfd;
    socket = rfs->socket;

    phead_t *h = rfs_protocol_new_head(RFS_READ, rfs);
    h->size = sizeof(rfs_read_in_t);

    rfs_read_in_t *idata = calloc(h->size, 1);
    idata->ctx = rfs->ctx;
    idata->count = count;

    if(write(socket, h, sizeof(phead_t))!=sizeof(phead_t))
      ABORT_ME("%s: Fail to send protocol header\n", __func__);

    if(write(socket, idata, sizeof(rfs_read_in_t))!=sizeof(rfs_read_in_t))
      ABORT_ME("%s: Fail to send payload\n", __func__);
    free(idata);

    if(write(socket, buf, count)!=count)
      ABORT_ME("%s: Fail to send second half of payload\n", __func__);

    //get information of closed file
    if(read(socket, h, sizeof(phead_t))!=sizeof(phead_t))
      ABORT_ME("%s: Fail to recv protocol header\n", __func__);

    rfs_read_ou_t *odata = calloc(h->size, 1);
    if(read(socket, odata, h->size)!=h->size)
      ABORT_ME("%s: Fail to recv payload\n", __func__);

    ret = odata->ret;
    errno = odata->rerrno;
    free(odata);
    free(h);
    return ret;
}
