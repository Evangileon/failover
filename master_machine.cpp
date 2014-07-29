
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <iostream>
#include <string>

#include "config.h"
#include "util.h"
#include "net_util.h"
#include "master_machine.h"
#include "status_check.h"
#include "async_handle_asterisk.h"

sem_t status_send_end_sem;
sem_t status_recv_end_sem;

int needStatusSend = 1;

int master_status_send_loop(int wfd) {
    fd_set wfds; 
    int ret = -1;
    char buffer[SEND_BUFFER_SIZE];

    int restartCount = 0;
    bool end_status_send_loop = false;
    int loop_ret_val = 0;

    while(1) {
        sleep(1);
        if(!needStatusSend) {
            break;
        }

        int check = checkStatus();
        std::string whatToSend = checkResultStr(check);

        if(check == ASTERISK_STOP) { // "down"
            async_handle_asterisk::restart();
            restartCount++;
            if(restartCount == MAX_RESTART_TIMES) {
                whatToSend = "down";
                restartCount = 0;
                async_handle_asterisk::stop();
                end_status_send_loop = true;
            } else {
                whatToSend = "master try to restart asterisk";
            }
        }

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

        ret = send(wfd, buffer, whatToSend.length(), MSG_NOSIGNAL);
        DEBUG("errno = %d\n", errno);
        if(ret != whatToSend.length()) {
            break;
        }
        std::cout << "status sent\n";
        if (end_status_send_loop) {
            loop_ret_val = ASTERISK_STOP;
            break;
        }
    }

    return loop_ret_val;
}

void * master_status_send(void *exit_val) {
    
    int sockfd;
    int thread_exit_val = 0;

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
        int status = master_status_send_loop(sockfd);
        if(status == ASTERISK_STOP) {
            std::cout << "cease status sending\n";
            thread_exit_val = status;
            close(sockfd);
            break;
        }
        close(sockfd);
    }

    *((int *)exit_val) = thread_exit_val;
    return NULL;
}

void master_machine() {
    int ret;
    int exit_val = 0;
    pthread_t master_thread;
    struct thread_info *tinfo;
    pthread_attr_t attr;

    if((ret = pthread_attr_init(&attr)) != 0) {
        ERROR("Create init thread attr error: %d\n", ret);
    }

	if((ret = sem_init(&status_send_end_sem, 0, 0)) != 0) {
        ERROR("semaphore init thread attr error: %d\n", ret);
    }

    if((ret = pthread_create(&master_thread, &attr, &master_status_send, (void *)&exit_val)) != 0) {
        ERROR("Create thread error: %d\n", ret);
    }

	//system("/sbin/ifup eth2");
	
}