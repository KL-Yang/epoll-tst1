#include "../common/rfs_i.h"
#include <aio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
//#include <uv.h>
#include <glib.h>

#include <sys/epoll.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define SVR_MAX_EVENT       16
#define SVR_MAX_CLIENT      128

//protocol command
#define PCMD_MODE_DATA      (1u<<0)     //send/recv data, other wise head

typedef struct cln_ctx_s cln_ctx_t;

typedef struct {
    int32_t         flag;
    int32_t        _reserved;
    int64_t         offset;
    cln_ctx_t      *ctx;
    phead_t         head;       //function id
    void           *data;
} rfs_cmd_t;

/**
 * create on connect accept, destroy on connection lost
 * */
#define SVR_CLIENT_QD   1
typedef struct {

    int                 efd;            /* epoll fd */
    int                 socket;         /* listening socket */
    struct epoll_event *event;

    pthread_spinlock_t  lock;
    GThreadPool        *cmd_pool;       /* thread pool to handle commands */
    GList              *cln_list;
    GList              *lfd_list;       /* if client lost, lfd wait here for reconnection */
                     
} svr_ctx_t;

typedef struct cln_ctx_s {

    int                 socket;
    pthread_spinlock_t  lock;
    svr_ctx_t          *svr;
    rfs_cmd_t          *cmd;

    rfs_cmd_t          *ret;
    GQueue             *ret_que;
    int                 rqd;

} cln_ctx_t;

/**
 * the client side fd should be a pointer to lfd_ctx_t struct!
 * create on RFS_OPEN, destroy on RFS_CLOSE
 * WATCHOUT: we don't maintain the queue for simplicity
 *           use block io for file!
 * */
#define RFS_LFD_CLOSE       (1u<<1)     /* marked for close! */

typedef struct {

    int             fd;
    int             magic;
    int             flag;
    int             sequence;
    cln_ctx_t      *cln;
    //add time stamp of last active command

} lfd_ctx_t;

void server_dispatch(void *data, void *user_data);

lfd_ctx_t * svr_rfs_open(rfs_open_in_t *in, void **ppou);
lfd_ctx_t * svr_rfs_close(rfs_close_in_t *in, void **ppou);
void svr_rfs_read(rfs_read_in_t *in, void **ppou);
void svr_rfs_write(rfs_write_in_t *in, void **ppou);


svr_ctx_t * svr_ctx_init(const char *host, int port, int nthread);
void svr_run_loop(svr_ctx_t *svr);
void svr_ctx_free(svr_ctx_t *svr);

cln_ctx_t * svr_new_client(int socket, svr_ctx_t *svr);

int svr_accept(svr_ctx_t *svr);
int svr_inbound(cln_ctx_t *cln);
int svr_outbound(cln_ctx_t *cln);

