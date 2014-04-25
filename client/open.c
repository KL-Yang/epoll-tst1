#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libsoup/soup.h>
#include "rfs_c.h"

phead_t * 
rfs_protocol_new_head(int func_id, rfs_t *rfs)
{ 
    phead_t *h;
    h = calloc(1, sizeof(phead_t));
    memcpy(h->protocol, "RFS\0", 4*sizeof(char));
    h->func_id = func_id;
    if(rfs!=NULL) {
        h->ctx = rfs->ctx;
        h->magic = rfs->magic;
        h->sequence = rfs->sequence;
        rfs->sequence++;
    }
    return h;
}

int64_t rfs_open(const char *pathname, int flags)
{
    //decompose uri
    SoupURI *soup = soup_uri_new(pathname);
    printf("soup scheme=%s\n", soup->scheme);
    printf("soup host=%s\n", soup->host);
    printf("soup port=%d\n", soup->port);
    printf("soup user=%s\n", soup->user);
    printf("soup path=%s\n", soup->path);

    //make connection to server!
    rfs_t *rfs = calloc(1, sizeof(rfs_t));
    rfs->magic = random();
    rfs->sequence = 1; //sequence 0 reserved authorization! 

    //build the head
    phead_t *h = rfs_protocol_new_head(RFS_OPEN, rfs);
    h->size = sizeof(rfs_open_in_t)+(strlen(soup->path)+sizeof(char));

    //build the data
    rfs_open_in_t *idata = calloc(h->size, 1);
    idata->flags = flags;
    strcpy(idata->pathname, soup->path);

    //open socket and send head/data
    int socket;
    socket = rfs_socket_connect("localhost", (int)soup->port);
    rfs->socket = socket;

    if(write(socket, h, sizeof(phead_t))!=sizeof(phead_t))
      ABORT_ME("%s: Failed to send protocol header\n", __func__);

    if(write(socket, idata, h->size)!=h->size)
      ABORT_ME("%s: Failed to send payload\n", __func__);

    //get information of opened file
    if(read(socket, h, sizeof(phead_t))!=sizeof(phead_t))
      ABORT_ME("%s: Failed to recv protocol header\n", __func__);

    rfs_open_ou_t *odata = calloc(h->size, 1);
    if(read(socket, odata, h->size)!=h->size)
      ABORT_ME("%s: Failed to recv payload\n", __func__);

    rfs->ctx = odata->ctx;
    if(rfs->ctx==0) {
        fprintf(stderr, "%s: failed %s\n", __func__, strerror(odata->rerrno));
        abort();
    }

    fprintf(stderr, "%s(%s)=%ld\n", __func__, idata->pathname, rfs->ctx);
    soup_uri_free(soup);
    free(idata);
    free(odata);
    free(h);

    return (int64_t)rfs;
}
