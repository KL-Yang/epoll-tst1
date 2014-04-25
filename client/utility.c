#include "rfs_c.h"

int client_send_cmd(int socket, phead_t *h, void *pay)
{
    ssize_t count;

    do {
        count = write(socket, h, sizeof(phead_t));
    } while(count==-1 && errno==EINTR);

    if(count!=sizeof(phead_t)) {
      ABORT_ME("%s: Fail to send protocol header\n", __func__);
    }

    do {
        count = write(socket, pay, h->size);
    } while(count==-1 && errno==EINTR);

    if(count!=h->size) {
      ABORT_ME("%s: Fail to send payload\n", __func__);
    }

    return 0;
}

int client_recv_ret(int socket, phead_t *h, void *pay)
{
    ssize_t count;
    void **payload = (void**)pay;

    do {
        count = read(socket, h, sizeof(phead_t));
    } while(count==-1 && errno==EINTR);

    if(count!=sizeof(phead_t)) {
      ABORT_ME("%s: Fail to recv protocol header\n", __func__);
    }

    *payload = malloc(h->size); assert(*payload!=NULL);

    do {
        count = read(socket, *payload, h->size);
    } while(count==-1 && errno==EINTR);

    if(count!=h->size) {
      ABORT_ME("%s: Fail to recv payload\n", __func__);
    }

    return 0;
}
