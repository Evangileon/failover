
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#include "thread_util.h"

void cleanup_handler(void *pfd) {
    int fd = (long)pfd;
    if(fd < 0) {
        return;
    }

    int ret;
    if((ret = close(fd)) != 0) {
        if(errno == EBADF) {
            return;
        }
        perror("cleanup: ");
    }
}
