#include "../common/rfs_i.h"
#include <aio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <uv.h>
#include <glib.h>

//protocol command
#define PCMD_MODE_DATA      (1u<<0)     //send/recv data, other wise head

typedef struct {
    int32_t         flag;
    int32_t        _reserved;
    int64_t         offset;
    phead_t         head;       //function id
    void           *data;
} rfs_cmd_t;

typedef struct {
    uv_write_t      req[2];
    uv_buf_t        buf[2];
    rfs_cmd_t      *cmd;
} rfs_ret_t;

/**
 * create on connect accept, destroy on connection lost
 * */
typedef struct {

    uv_loop_t          *loop;
    int                 bcon;

    rfs_cmd_t          *cmd;            /* TODO: move to cln_ctx_t on command received, append to lfd_ctx queue */
    GThreadPool        *cmd_pool;       /* thread pool to handle commands */

    pthread_spinlock_t  lock;
    GList              *cln_list;
    GList              *lfd_list;       /* TODO: move to cln_ctx_t local file associate with this socket! */
                     
    GQueue             *ret_que;        /* return queue, when lfd finished, append to this queue */
    //rfs_cmd_t          *ret;
    int                 rqd;            /* return queue depth */

    uv_stream_t        *client;         /* TODO move to cln_ctx_t */
    uv_stream_t        *trigger;

} svr_ctx_t;

typedef struct {

    svr_ctx_t          *svr;
    rfs_cmd_t          *cmd;
    GList              *lfd_list;
    uv_stream_t        *client;

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
    //add time stamp of last active command

} lfd_ctx_t;

void beacon_connection(uv_stream_t *server, int status);

svr_ctx_t * rfss_new_context(uv_stream_t *client);
void server_dispatch(void *data, void *user_data);

lfd_ctx_t * svr_rfs_open(rfs_open_in_t *in, void **ppou);
lfd_ctx_t * svr_rfs_close(rfs_close_in_t *in, void **ppou);

void svr_rfs_read(rfs_read_in_t *in, void **ppou);
void svr_rfs_write(rfs_write_in_t *in, void **ppou);
