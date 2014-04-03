#include "../common/rfs_i.h"

typedef struct {
    int64_t     rfp;        /* remote file context pointer */
    int         socket;
    int         magic;      /* magic associate with fd */
    int         sequence;   /* call count */
    int         flag;
    GQueue    * cmd;

/////////////to do, remove
    GQueue    * rque;       /* read queue of rcb */
    GQueue    * wque;       /* write queue of rcb */
//example: http://www.wangafu.net/~nickm/libevent-book/01_intro.html
} rfd_t;

typedef struct {
    rfd_t     * rfd;
    phead_t   * head;
    void      * idat;
    void      * odat;
    int         magic;
    int         sequence;
    int         sem;        //-1 for no sem
} rcb_t;

int64_t rfs_open(const char *pathname, int flags);
int rfs_close(int64_t rfh);
////////////////////////

int64_t rfs_fd_new();
int rfs_fd_getfd(int64_t rfd);
rfd_t * rfs_fd_getctr(int64_t rfd);
void rfs_fd_del(int64_t rfd);

int rfs_socket_connect(const char *host, int port);
phead_t * rfs_protocol_new_head(int func_id, rfd_t *ctr);
ssize_t rfs_pwrite(int64_t rfh, const void *buf, size_t count, off_t offset);


