#include "rfs_c.h"

/**
 * connet to remote server and return socket fd.
 * if authorization is required, add in this function.
 * */
int rfs_socket_connect(const char *host, int port)
{
    int fd;
    struct hostent *hent;
    struct sockaddr_in addr;

    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0) {
        fprintf(stderr, "Failed to create socket\n");
        abort();
    }
    hent = gethostbyname(host);
    memset(&addr, 0, sizeof(addr)); 
    memcpy((char*)&addr.sin_addr, hent->h_addr, hent->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);      // Set port number
    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr))<0) {
        fprintf(stderr, "\n Error : Connect Failed \n");
        return 1;
    }
    return fd;
}
