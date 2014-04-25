#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#define RFS_PORT            (2048)

#define RFS_SOCK_CLIENT     (0)         //dummy
#define RFS_SOCK_SERVER     (1<<0)
#define RFS_SOCK_NONBLOCK   (1<<1)

#define RFS_SOCK_BACKLOG    (10)

#define ABORT_ON(cond, ...) { if(cond) { fprintf(stderr, __VA_ARGS__); abort(); } }
#define ABORT_ME(...)       ABORT_ON(1, __VA_ARGS__)

typedef struct  //remote call protocal header
{
    char        protocol[4];    //RFS0
    int32_t     magic;          //random number associated with rctx
    int64_t     ctx;            //handle of remote context, can be NULL
    int32_t     func_id;
    int32_t     sequence;       //each call over socked +1
    int64_t     size;           //data package size
    int32_t     c_magic;        //connection magic word
    int32_t     c_sequence;     //connection packet sequence.
    int32_t    _reserved[6];   
} phead_t;      //64Byte

typedef struct
{
    int64_t     mode;
    int64_t     size;
} rfs_stat_t;

#define RFS_STAT_DIR        1
#define RFS_STAT_PATH       1

#define RFS_ISREG(pstat)     (((pstat)->mode)==0)               //is regular file?
#define RFS_ISDIR(pstat)     (((pstat)->mode)&RFS_STAT_DIR)     //is directory?


enum func_list 
{
    RFS_OPEN    = 1,            //mask by 0x000000FF, each group has maximum 127 function
    RFS_CLOSE,

    RFS_READ,
    RFS_PREAD,
    RFS_WRITE,
    RFS_PWRITE,
    RFS_FUNC_STAT,                   //struct stat is not portable, we will define a portable one
    RFS_FSTAT,                  //
    RFS_SEEK,

    //>>>>>>>>>>>>>>>>>>>>>>>
    RDB_OPEN    = 257,
    RDB_CLOSE,
    RDB_INIT,           /* for read, init the db structure, for create also */
/*    
    RDB_NFIELD,         //those can be implemented on client side!
    RDB_NINST,          //should also include a symbol talbe for global property!
    RDB_SETFIELD,
    RDB_GETFIELD,
    RDB_SETPROPERTY,
    RDB_GETPROPERTY,
*/    
    RDB_READ,
    RDB_WRITE,
};

int rfs_sock_connect(const char *host, int port, int flag);
int rfs_sock_nonblock(int fd);

//<<rfs_open()
typedef struct 
{
    int64_t         flags;
    char            pathname[];
} rfs_open_in_t;

typedef struct
{
    int64_t         ctx;
    int64_t         rerrno;
} rfs_open_ou_t;

//<<rfs_close()
typedef struct 
{
    int64_t         ctx;
} rfs_close_in_t;

typedef struct 
{
    int64_t         ret;        /* return value */
    int64_t         rerrno;     /* remote errno */
} rfs_close_ou_t;

//<<rfs_write(), rfs_pwrite(), rfs_swrite all use this struct!
typedef struct 
{
    int64_t         ctx;
    int64_t         count;
    int64_t         offset;
    int64_t        _reserved;
    char            buf[];
} rfs_write_in_t;

typedef struct
{
    int64_t         ret;
    int64_t         rerrno;
} rfs_write_ou_t;

typedef struct
{
    int64_t         ctx;
    int64_t         count;
    int64_t         offset;
    int64_t        _reserved;
} rfs_read_in_t;

typedef struct 
{
    int64_t         ret;
    int64_t         rerrno;
    int64_t        _reserved[2];
    char            buf[];
} rfs_read_ou_t;

typedef struct 
{
    int32_t         flag;
    int32_t        _reserved;
    int64_t         rfd;        //remote file descriptor context
    char            path[];
} rfs_stat_in_t;

typedef struct 
{
    int32_t         ret;
    rfs_stat_t      stat;
    int64_t         rerrno;
} rfs_stat_ou_t;
