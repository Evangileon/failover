
#include <iostream>

#include "util.h"
#include "statusCheck.h"

using namespace std;
int needStatusSend = 0;

int master_status_send_loop(int wfd) {
    fd_set wfds; 
    int ret = -1;
    char buffer[16];

    while(1) {
        sleep(1);
        if(!needStatusSend) {
            break;
        }
        ret = select_write_with_timeout(wfd, &wfds, 5);
        if(ret <= 0) {
            break;
        }
        if(!FD_ISSET(wfd, &wfds)) {
            break;
        }

        ret = send(wfd, buffer, 16, MSG_NOSIGNAL);
        DEBUG("errno = %d\n", errno);
        if(ret != 16) {
            break;
        }
        cout << "send succeed\n";
    }

    return ret;
}

void master_status_send() {
    int wfd;
    int sockfd;
    

    struct timeval timeout;
    timeout.tv_sec = 5;  // It matter on the interval of heartbeat
    timeout.tv_usec = 0;

    struct sockaddr_in receiver_addr;
    bzero((void *)&receiver_addr, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = inet_addr(STANDBY_ADDR);
    receiver_addr.sin_port = htons(STANDBY_PORT);

    while(1) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd < 0) {
            ERROR("%d\n", __LINE__);
        }
        
        //if(setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(int)) < 0 ) {
        //    perror("setopt send");
        //    ERROR("%d\n", __LINE__);
        //}
        //
        int set = 1;
        /*if(setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)) < 0) {
            perror("setsock  nopipe");
            ERROR("%d\n", __LINE__);
        }*/
        
        int ret = connect_nonblock(&receiver_addr, sockfd, 5);
        if(ret < 0) {
            if(errno == ECONNREFUSED) {
                cout << "connection refused\n";          
                sleep(1);
                continue;
            }

            perror("connect after nonblock");
            ERROR("%d\n", __LINE__);
        }

        if(errno == ETIMEDOUT) {
            // time out
            continue;
        }
        
        cout << "go to send loop\n";
        master_status_send_loop(sockfd);
        cout << "cease sending\n";
        close(sockfd);
    }
}