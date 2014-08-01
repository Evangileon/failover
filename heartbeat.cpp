#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>

#include <thread>
#include <memory>

#include "util.h"
#include "net_util.h"
#include "config.h"
#include "async_handle_asterisk.h"
#include "master_machine.h"
#include "heartbeat.h"



int heartbeat::heartbeat_receive_loop(int rfd) {
    fd_set rfds;
    int ret = -1;
    char buffer[16];

    while(1) {
        ret = select_with_timeout(rfd, &rfds, 2);
        if(ret <= 0) {
            break;
        }
        if(!FD_ISSET(rfd, &rfds)) {
            break;
        }

        ret = recv(rfd, buffer, 16, 0);
        if(ret != 16) {
            break;
        }
        std::cout << "heartbeat recv succeed\n";
        sleep(1);
    }

    return ret;
}

int heartbeat::heartbeat_send_loop(int wfd) {
    fd_set wfds; 
    int ret = -1;
    char buffer[16];

    while(1) {
        sleep(1);
        if(!needSend) {
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
        std::cout << "heartbeat send succeed\n";
    }

    return ret;
}

void heartbeat::heartbeat_receive() {
    int sockfd;
    int ret;
    int status = 0;
    fd_set rfds;

    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    cli_len = sizeof(cli_addr);

    sockfd = get_any_tcp_connection_ready(HEARTBEAT_RECEIVE_PORT, MAX_CONN_COUNT);
    if ((sockfd) < 0) {
        perror("ERROR opening socket");
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
            needSend = 0;
            std::cout << "The other is dead\n";
        } else {
            // nothing to do
            std::cout << "go to heartbeat receive loop\n";
            int cfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
            if(cfd < 0) {
                perror("It shouldn't be error");
            }
            needSend = 1;
            heartbeat_receive_loop(cfd);
            close(cfd);
        }
        if(status < 0) {
        }
    }
}


void heartbeat::heartbeat_send() {
    
    int sockfd;
   
    struct sockaddr_in receiver_addr;
    bzero((void *)&receiver_addr, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    receiver_addr.sin_port = htons(HEARTBEAT_RECEIVE_PORT);

    while(1) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd < 0) {
            ERROR("%d\n", __LINE__);
        }
        
        int ret = connect_nonblock(&receiver_addr, sockfd, 5);
        if(ret < 0) {
            if(errno == ECONNREFUSED) {
                std::cout << "connection refused\n";          
                sleep(1);
                continue;
            }

            perror("connect after nonblock");
            //ERROR("%s:%d\n", __FILE__, __LINE__);
        }

        if(errno == ETIMEDOUT) {
            // time out
            continue;
        }
        
        std::cout << "go to send loop\n";
        heartbeat_send_loop(sockfd);
        std::cout << "cease sending\n";
        close(sockfd);
    }
}

void heartbeat::start_heartbeat_send() {
    
}

void heartbeat::start_heartbeat_recv() {
   
}


std::auto_ptr<heartbeat> init_heartbeat() {
    heartbeat *hb = new heartbeat();
    std::thread hbRecv(&heartbeat::heartbeat_send, hb);
    hbRecv.detach();
    sleep(4);
    std::thread hbSend(&heartbeat::heartbeat_receive, hb);
    hbSend.detach();

    std::auto_ptr<heartbeat> hb_ptr(hb);
    return hb_ptr;
}