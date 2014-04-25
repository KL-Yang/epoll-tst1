
/**
 * use a hash table to manage connections
 *
 * "host"  ->  "context structure"   Port socket! 
 *
 * */
#include "rfs_c.h"
#define _GNU_SOURCE
#include <search.h>

//maximum connection to hosts
#define RFS_MAX_CLIENT_CONNECTION   128

static int ctx_count;
static struct hsearch_data ctx_hosts;

int hcreate_r(size_t nel, struct hsearch_data *htab);

int cln_init()
{
    int ret;
    assert(ctx_count==0);
    ret = hcreate_r(RFS_MAX_CLIENT_CONNECTION, &ctx_hosts);
    if(ret!=0) {
        fprintf(stderr, "<%s> failed hcreate_r() to create connection table! "
                "ERROR: %s\n", __func__, strerror(errno));
        abort();
    }
    return 0;
}

lib_ctx_t *
cln_ctx_init(const char *host, int port)
{
    lib_ctx_t *ctx = calloc(1, sizeof(lib_ctx_t));
    ctx->socket = rfs_sock_connect(host, port, RFS_SOCK_CLIENT);
    if(ctx->socket < 0) {
        fprintf(stderr, "%s: failed to connection!\n", __func__);
        abort();
    }
    ctx->ctime = time(&ctx->atime);
    return ctx;
}

/**
 * connect if no connection available
 * */
lib_ctx_t *
cln_get_context(const char *host, int port)
{
    int ret;
    lib_ctx_t *ctx;
    ENTRY rinfo, *pinfo;

    if(ctx_count==0)
      ret = cln_init();

    rinfo.key  = (char*)host;
    rinfo.data = NULL;      //should be a pointer to local context!!!
    if(hsearch_r(rinfo, FIND, &pinfo, &ctx_hosts)) {
        ctx = (lib_ctx_t*)(pinfo->data);
        assert(ctx!=NULL);
        ctx->rfd_count++;
        return ctx;
    } else if(errno == ESRCH) {
        ctx = cln_ctx_init(host, port);
        rinfo.key  = (char*)host;
        rinfo.data = ctx;
        ret = hsearch_r(rinfo, ENTER, &pinfo, &ctx_hosts);
        ctx_count++;
        assert(ret == 0);
        return ctx;
    }
    return NULL;
}

/**
 * call this periodically to cleanup dead connections.
 * */
void *
cln_ctx_clean()
{
    return NULL;
}
