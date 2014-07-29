#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <string>
#include <thread>

#include "config.h"
#include "util.h"
#include "net_util.h"
#include "standby_machine.h"
#include "master_machine.h"
#include "status_check.h"

int status_receive_loop(int rfd) {
    fd_set rfds;
    int ret = -1;
    char buffer[SEND_BUFFER_SIZE];


    while(1) {
        ret = select_with_timeout(rfd, &rfds, 2);
        if(ret <= 0) {
            break;
        }
        if(!FD_ISSET(rfd, &rfds)) {
            break;
        }

        ret = recv(rfd, buffer, SEND_BUFFER_SIZE, 0);
        if(ret <= 0) {
            break;
        }
        if(ret > SEND_BUFFER_SIZE) {
        	std::cout << "I am hacked" << std::endl;
        	ret = -1;
        	break;
        }

        buffer[ret] = '\0';

        std::cout << "master is " << buffer << std::endl;
        if(strcmp(buffer, "down") == 0) {
            ret = MASTER_ASTERISK_STOP;
            break;
        }
        sleep(1);
    }

    std::cout << "End status receive loop" << std::endl;

    return ret;
}

void * status_receive(void *exit_val) {
    int sockfd;
    int ret;
    int status = 0;
    int thread_exit_val;
    fd_set rfds;

    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    cli_len = sizeof(cli_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ((sockfd) < 0) {
        perror("ERROR opening socket");
        ERROR("%d\n", __LINE__);
    }

    int net_optval = 1;
    if(setsockopt((sockfd), SOL_SOCKET, SO_REUSEADDR, &net_optval, sizeof net_optval) < 0) {
        perror("errno on setsockopt");         
        ERROR("%d\n", __LINE__);    
    }  

    struct sockaddr_in net_serv_addr;
    bzero((void *) &net_serv_addr, sizeof(net_serv_addr));
    net_serv_addr.sin_family = AF_INET;
    net_serv_addr.sin_addr.s_addr = INADDR_ANY;
    net_serv_addr.sin_port = htons((STATUS_PORT));
    if (bind((sockfd), (struct sockaddr *) &net_serv_addr,sizeof(net_serv_addr)) < 0) {
        perror("ERROR on binding, receiveMessage");
        ERROR("%d\n", __LINE__);
    }
    if(listen((sockfd),(MAX_CONN_COUNT)) < 0) {
        perror("listen, receiveMessage");
        ERROR("%d\n", __LINE__);
    }

    
    while(1) {
        //sockfd = get_any_tcp_connection_ready_socket(sockfd, HEARTBEAT_RECEIVE_PORT, MAX_CONN_COUNT);
        if(sockfd < 0) {
            ERROR("%s, %d\n", __FILE__, __LINE__);
        }

        ret = select_with_timeout(sockfd, &rfds, 5);
        if(ret < 0) {
            perror("after select");
            ERROR("%s, %d\n", __FILE__, __LINE__);
        } else if (ret == 0) {
            // timeout
            // the other one is dead
            status = -1;
            needStatusSend = 0;
            std::cout << "Not receive status from master\n";
        } else {
            // nothing to do
            std::cout << "status receive loop\n";
            int cfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
            if(cfd < 0) {
                perror("It shouldn't be error");
            }
            needStatusSend = 1;
           	int masterStatus = status_receive_loop(cfd);
            if(masterStatus == MASTER_ASTERISK_STOP) {
                close(cfd);
                thread_exit_val = MASTER_ASTERISK_STOP;
                break;
            }
            
            close(cfd);
        }
    }
    std::cout << "status receive thread end" << std::endl;
    *((int *)exit_val) = thread_exit_val;
    return NULL;
}



int standby_machine() {
	system("/sbin/ifdown eth2");
	system("/etc/init.d/asterisk stop");
	
	int ret;
    int exit_val = 0;
    pthread_t standby_thread;
    struct thread_info *tinfo;
    pthread_attr_t attr;

    if((ret = pthread_attr_init(&attr)) != 0) {
        ERROR("Create init thread attr error: %d\n", ret);
    }

    if((ret = sem_init(&status_send_end_sem, 0, 0)) != 0) {
        ERROR("semaphore init thread attr error: %d\n", ret);
    }

    if((ret = pthread_create(&standby_thread, &attr, &status_receive, (void *)&exit_val)) != 0) {
        ERROR("Create thread error: %d\n", ret);
    }

    if((ret = pthread_join(standby_thread, NULL)) != 0) {
        perror("Can not join");
    }

    return exit_val;
}