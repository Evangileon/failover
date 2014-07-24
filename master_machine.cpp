
#include <thread>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "config.h"
#include "util.h"
#include "net_util.h"
#include "master_machine.h"
#include "status_check.h"

sem_t status_send_end_sem;
sem_t status_recv_end_sem;

int needStatusSend = 1;

int master_status_send_loop(int wfd) {
    fd_set wfds; 
    int ret = -1;
    char buffer[16];

    while(1) {
        sleep(1);
        if(!needStatusSend) {
            break;
        }

        std::string whatToSend = checkResultStr();
		whatToSend.copy(buffer, whatToSend.length( ));
		buffer[whatToSend.length()] = '\0';
		std::cout << "message: " << buffer << std::endl;

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
        std::cout << "status sent\n";
    }

    return ret;
}

void master_status_send() {
    
    int sockfd;

    struct sockaddr_in receiver_addr;
    bzero((void *)&receiver_addr, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = inet_addr(STANDBY_ADDR);
    receiver_addr.sin_port = htons(STATUS_PORT);

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
        /*if(setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)) < 0) {
            perror("setsock  nopipe");
            ERROR("%d\n", __LINE__);
        }*/
        
        int ret = connect_nonblock(&receiver_addr, sockfd, 5);
        if(ret < 0) {
            if(errno == ECONNREFUSED) {
                std::cout << "connection refused\n";          
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
        
        std::cout << "status send loop\n";
        master_status_send_loop(sockfd);
        //cout << "cease sending\n";
        close(sockfd);
    }
}

void master_machine() {

	sem_init(&status_send_end_sem, 0, 0);
	//system("/sbin/ifup eth2");
	std::thread statusSend(master_status_send);
	statusSend.detach();
	
}