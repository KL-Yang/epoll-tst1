#include "rfs_c.h"

int main()
{
    int64_t fd;
    char *buf="This is a hello Wrold!";
    char *uri="rfs://kyang@pt001:2048/home/kyang/test.dat";

    fd = rfs_open(uri, O_RDWR | O_TRUNC | O_CREAT | S_IWUSR | S_IRUSR);
    rfs_write(fd, buf, strlen(buf));
    rfs_close(fd);

    return 0;
}
