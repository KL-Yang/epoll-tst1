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

/**
 * create on connect accept, destroy on connection lost
 * */
typedef struct {

    int                 socket;

    rfs_cmd_t          *cmd;            /* on command received, append to lfd_ctx queue */
    GThreadPool        *cmd_pool;       /* thread pool to handle commands */
    int                 cqd;            /* command queue depth */

    pthread_spinlock_t  lock;
    GList              *lfd_list;       /* local file associate with this socket! */
                     
    GQueue             *ret_que;        /* return queue, when lfd finished, append to this queue */
    rfs_cmd_t          *ret;
    int                 rqd;            /* return queue depth */

} svr_ctx_t;

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

svr_ctx_t * rfss_new_context();
void server_dispatch(void *data, void *user_data);

lfd_ctx_t * svr_rfs_open(rfs_open_in_t *in, void **ppou);
lfd_ctx_t * svr_rfs_close(rfs_close_in_t *in, void **ppou);

void svr_rfs_read(rfs_read_in_t *in, void **ppou);
void svr_rfs_write(rfs_write_in_t *in, void **ppou);
/*

rfs_ctx_t * rfs_ctx_create(struct event_base *base, evutil_socket_t fd);

int do_cmd_head(evutil_socket_t socket, rfs_cmd_t *cmd, char type);
int do_cmd_data(evutil_socket_t socket, rfs_cmd_t *cmd, char type);
void do_accept(evutil_socket_t listener, short event, void *arg);
void rfs_ctx_recv(evutil_socket_t fd, short events, void *arg);
void rfs_ctx_send(evutil_socket_t fd, short events, void *arg);
void rfs_svr_open(rfs_ctx_t *ctx, rfs_cmd_t *cmd);
void rfs_lfd_read(evutil_socket_t fd, short events, void *arg);
void rfs_lfd_write(evutil_socket_t fd, short events, void *arg);
void rfs_verify_cmd(rfs_ctx_t *ctx, rfs_cmd_t *cmd);
void rfs_svr_read_attach(rfs_ctx_t *ctx, rfs_cmd_t *cmd);
void rfs_svr_write_attach(rfs_ctx_t *ctx, rfs_cmd_t *cmd);
void rfs_svr_mark_close(rfs_ctx_t *ctx, rfs_cmd_t *cmd);
void rfs_svr_send_attach(rfs_ctx_t *ctx, rfs_cmd_t *cmd);
*/
