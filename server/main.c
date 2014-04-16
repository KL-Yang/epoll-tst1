#include "rfs_s.h"

int main() 
{
    //mainly to handle command line arguments!

    int nthread;
    svr_ctx_t *svr;

    nthread = 2;
    svr = svr_ctx_init("127.0.0.1", RFS_PORT, nthread);

    svr_run_loop(svr);

    svr_ctx_free(svr);

    return 0;
}
