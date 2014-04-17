#include "../common/rfs_i.h"

typedef struct {
    int64_t     ctx;        /* remote file context pointer */
    int         magic;      /* magic associate with fd */
    int         sequence;   /* call count */
    int         flag;
    int         socket;
} rfs_t;    //local

int64_t rfs_open(const char *pathname, int flags);
int64_t rfs_write(int64_t rfd, const void *buf, int64_t count);
int64_t rfs_read(int64_t rfd, void *buff, int64_t count);
int     rfs_close(int64_t rfh);
////////////////////////

int rfs_socket_connect(const char *host, int port);
phead_t * rfs_protocol_new_head(int func_id, rfs_t *ctr);
ssize_t rfs_pwrite(int64_t rfh, const void *buf, size_t count, off_t offset);

