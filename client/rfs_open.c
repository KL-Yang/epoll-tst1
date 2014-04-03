#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <assert.h>
#include <libsoup/soup.h>

static uv_loop_t *uv_loop = NULL;
static pthread_mutex_t uv_lock = PTHREAD_MUTEX_INITIALIZER;

phead_t * 
rfs_protocol_new_head(int func_id, rfd_t *ctr)
{ 
    phead_t *h;
    h = calloc(1, sizeof(phead_t));
    memcpy(h->ver, "RFS0", 4*sizeof(char));
    h->lfd = ctr->rfp;
    h->magic = ctr->magic;
    h->func_id = func_id;
    h->sequence = ctr->sequence;
    ctr->sequence++;
    return h;
}

int64_t rfs_open(const char *pathname, int flags)
{
    //create loop if NULL
    pthread_mutex_lock(&uv_lock);
    if(uv_loop==NULL)
      uv_loop = uv_loop_new();
    pthread_mutex_unlock(&uv_lock);

    //decompose uri
    SoupURI *soup = soup_uri_new(pathname);
    printf("soup scheme=%s\n", soup->scheme);
    printf("soup host=%s\n", soup->host);
    printf("soup port=%d\n", soup->port);
    printf("soup user=%s\n", soup->user);
    printf("soup path=%s\n", soup->path);

    //make connection to server!
    uv_tcp_t client;
    uv_connect_t connect;
    struct sockaddr_in dest;
    uv_ip4_addr("127.0.0.1", 7000, &dest);

    rfd_t *rfd = calloc(1, sizeof(rfd_t));
    rfd->magic = random();
    rfd->sequence = 1; //sequence 0 reserved authorization! 
    rfd->cmd = g_queue_new();

    //build the head
    phead_t *h = rfs_protocol_new_head(RFS_OPEN, rfd);
    h->size = sizeof(rfs_open_in_t) + (strlen(soup->path)+1);

    //build the data
    rfs_open_in_t *idata = calloc(h->size, 1);
    idata->flags = flags;
    strcpy(idata->pathname, soup->path);

    //open socket and send head/data
    int socket;
    socket = rfs_socket_connect("localhost", (int)soup->port);

    ssize_t nreq;
    nreq = write(socket, h, sizeof(phead_t));
    assert(nreq==sizeof(phead_t));
    nreq = write(socket, idata, h->size);
    assert(nreq==h->size);

    //get information of opened file
    nreq = read(socket, h, sizeof(phead_t));
    assert(nreq==sizeof(phead_t));

    rfs_open_ou_t *odata = calloc(h->size, 1);
    odata = calloc(h->size, 1);
    nreq = read(socket, odata, h->size);
    assert(nreq==h->size);

    rfd->rfp = odata->rfd;
    rfd->socket = socket;

    fprintf(stderr, "%s(%s)=%ld\n", __func__, idata->pathname, rfd->rfp);
    soup_uri_free(soup);
    free(idata);
    free(odata);

    return (int64_t)rfd;
}
