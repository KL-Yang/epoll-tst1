#include "rfs_c.h"
#include <stdlib.h>
#include <argp.h>

const char *argp_program_version = "rfs_cp v0.01";
const char *argp_program_bug_address = "<yangkunlun@gmail.com>";

static char args_doc[] = " src_file des_file";

/* Program documentation. */
static char doc[] = "remote file system copy - a demonstrate client of "
"rfs library\nremote file format rfs://host/path/to/file";

/* The options we understand. */
static struct argp_option options[] = {
    {"verbose",  'v', 0,      0,  "Produce verbose output",     0},
    {"quiet",    'q', 0,      0,  "Don't produce any output",   0},
    {"silent",   's', 0,      OPTION_ALIAS,     0,              0},
    { 0 }
};

static struct argp argp = { options, 0, args_doc, doc, 0, 0, 0};


int main(int argc, char **argv)
{
    argp_parse (&argp, argc, argv, 0, 0, 0);
    int64_t fd;
    char *buf="This is a hello Wrold!\n";
    char *uri="rfs://kyang@pt001:2048/home/kyang/test.dat";

    fd = rfs_open(uri, O_RDWR | O_TRUNC | O_CREAT | S_IWUSR | S_IRUSR);
    rfs_write(fd, buf, strlen(buf));
    rfs_write(fd, buf, strlen(buf));
    rfs_write(fd, buf, strlen(buf));
    rfs_close(fd);

    return 0;
}
