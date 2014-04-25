#include "rfs_c.h"
#include <libsoup/soup.h>

/**
 * how to reuse the socket 
 * rfs_open("rfs://localhost/home/kyang/test.1")
 * rfs_open("rfs://localhost/home/kyang/test.2")
 * rfs_open("rfs://pts002/home/kyang/test.3")
 * ideally we only need open one socket to localhost and one to pts002
 *
 * build a hash table that map
 *  hostname -> client context ( socket )
 * */

int rfs_stat(const char *path, rfs_stat_t *stat)
{
    SoupURI *uri;
    lib_ctx_t *ctx;

    uri = soup_uri_new(path);
    ctx = cln_get_context(uri->host, uri->port);

    int r;
    phead_t *h;
    h = rfs_protocol_new_head(RFS_FUNC_STAT, NULL);
    h->size = sizeof(rfs_stat_in_t) + strlen(path) + 1;

    rfs_stat_in_t *idata;
    rfs_stat_ou_t *odata;

    idata = calloc(h->size, 1);
    idata->flag = idata->flag | RFS_STAT_PATH;
    strcpy(idata->path, path);

    r = client_send_cmd(ctx->socket, h, idata);
    assert(r == 0);

    r = client_recv_ret(ctx->socket, h, &odata);
    assert(r == 0);

    r = odata->ret;
    errno = odata->rerrno;
    memcpy(stat, &odata->stat, sizeof(rfs_stat_t));
    free(idata);
    free(odata);
    free(h);

    return r;
}
