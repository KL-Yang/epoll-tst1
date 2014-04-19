#include "rfs_s.h"

int svr_socket(const char *host, int port, int flag)
{
    int fd, try;
    struct hostent *hent;
    struct sockaddr_in addr;

       /* First call to socket() function */
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "ERROR opening socket: %s\n", strerror(errno));
        exit(1);
    }

    /* Initialize socket structure */
    hent = gethostbyname(host);
    memset(&addr, 0, sizeof(addr)); 
    memcpy((char*)&addr.sin_addr, hent->h_addr, hent->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);      // Set port number
 
    if(flag & SVR_SERVER) {
        if(bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            perror("ERROR on binding");
            exit(1);
        }
        if(listen(fd, SVR_BACKLOG) < 0) {
            perror("ERROR on listening");
            exit(1);
        }
    } else {
        try = 0;
        while(connect(fd, (struct sockaddr*)&addr, sizeof(addr))<0 
              && try < 10) {
            fprintf(stderr, "Fail to connect %s[%d]!\n", host, try);
            try++;
        }
    }

    if(flag & SVR_NONBLOCK)
      svr_make_non_block(fd);

    fprintf(stderr, "%s: fd=%d\n", __func__, fd);
    return fd;
}

int svr_make_non_block(int fd)
{
    int r, o_flag;
    if((o_flag = fcntl (fd, F_GETFL, 0))==-1) {
        perror ("fcntl");
        abort();
    }
    if((r = fcntl (fd, F_SETFL, o_flag | O_NONBLOCK)) == -1) {
        perror ("fcntl");
        abort();
    }
    return fd;
}
