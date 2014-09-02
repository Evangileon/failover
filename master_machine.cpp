#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <iostream>
#include <string>
#include <memory>

#include "config.h"
#include "util.h"
#include "net_util.h"
#include "master_machine.h"
#include "machine_interaction.h"
#include "status_check.h"
#include "async_handle_asterisk.h"
#include "thread_util.h"
#include "observer.h"

#include "config.h"

int needStatusSend = 1;

#define MAX_FD_NUM 16
int fds[MAX_FD_NUM];

master_machine::master_machine() {
    terminationFlag = 0;
    retval = 0;
}
master_machine::~master_machine() {
}

int master_machine::master_status_send_loop(int wfd) {
    fd_set wfds;
    int ret = -1;
    char buffer[SEND_BUFFER_SIZE];

    int restartCount = 0;
    bool end_status_send_loop = false;
    int loop_ret_val = 0;

    while (1) {

        if (terminationFlag) {
        	std::cout << "Termination flag = " << terminationFlag << std::endl;
            if (STANDBY_FAIL == terminationFlag) {
                std::cout << "standby is dead" << std::endl;
            }
        }

        sleep(1);
        if (!needStatusSend) {
        	std::cout << "No need to send status" << std::endl;
            break;
        }

        int check = checkStatus();
        std::string whatToSend = checkResultStr(check);

        if (check == ASTERISK_STOP) { // "down"
            async_handle_asterisk::restart();
            restartCount++;
            if (restartCount == MAX_RESTART_TIMES) {
                whatToSend = "down";
                restartCount = 0;
                std::cout << "Going to shut down asterisk on this machine" << std::endl;
                async_handle_asterisk::stop();
                end_status_send_loop = true;
            } else {
                whatToSend = "master try to restart asterisk";
            }
        }

        whatToSend.copy(buffer, whatToSend.length());
        buffer[whatToSend.length()] = '\0';
        std::cout << "message: " << buffer << std::endl;

        ret = select_write_with_timeout(wfd, &wfds, 5);
        if (ret <= 0) {
            break;
        }
        if (!FD_ISSET(wfd, &wfds)) {
            break;
        }

        ret = send(wfd, buffer, whatToSend.length(), MSG_NOSIGNAL);
        DEBUG("errno = %d\n", errno);
        if (ret != (int) whatToSend.length()) {
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

void master_machine::master_status_send() {

    int sockfd;
    int thread_exit_val = 0;

    char receiver_addr[20];
    std::string receiver_addr_s =
        config::instance().get_ip_master_status_send_to();
    int receiver_port = config::instance().get_port_status_receiver();
    receiver_addr_s.copy(receiver_addr, receiver_addr_s.length());
    unsigned timeout = config::instance().get_connect_nonblock_timeout();

    std::cout << "master status receiver addr: " << receiver_addr << ":"
              << receiver_port << std::endl;

    char sender_addr[20];
    std::string sender_addr_s = config::instance().get_ip_standby_status_recv();
    int sender_port = config::instance().get_port_status_sender();
    sender_addr_s.copy(sender_addr, sender_addr_s.length());

    std::cout << "master status sender bind to addr: " << sender_addr << ":"
              << sender_port << std::endl;

    while (1) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            ERROR("%d\n", __LINE__);
        }

        // ensure the fd will be closed while calling pthread_cancel
        pthread_cleanup_push(&cleanup_handler, (void *)((long)sockfd));

        if (config::instance().is_status_direct_link()) {
            enable_direct_link(sockfd);
        }

        sender_bind(NULL, sender_port, sockfd);

        int ret = connect_nonblock(receiver_addr, receiver_port, sockfd,
                                   timeout);
        if (ret < 0) {
            if (errno == ECONNREFUSED) {
                std::cout << "connection refused\n";
                sleep(1);
                continue;
            }

            perror("master status connect to other fail");
            printf("Requested address: %s:%d\n", receiver_addr, receiver_port);
            continue;
        }

        if (errno == ETIMEDOUT) {
            // time out
            continue;
        }

        std::cout << "status send loop\n";
        int status = master_status_send_loop(sockfd);
        if (status == ASTERISK_STOP) {
            std::cout << "cease status sending\n";
            thread_exit_val = MASTER_ASTERISK_STOP;
            close(sockfd);
            break;
        }
        pthread_cleanup_pop(1);
        close(sockfd);
    }

    //*((int *)exit_val) = thread_exit_val;
    retval = thread_exit_val;
    //return NULL;
}

void master_machine::inject_thread(std::thread &t) {
    master_thread.swap(t);
}

void master_machine::update(int flag) {

    switch (flag) {
    case THE_OTHER_IS_DEAD:
        terminationFlag = STANDBY_FAIL;
        break;
    case THE_OTHER_IS_ALIVE:
    	terminationFlag = STANDBY_ALIVE;
    	break;
    default:
        terminationFlag = 0;
    }
}

std::shared_ptr<master_machine> init_master_machine() {
    master_machine *mm = new master_machine();
    std::thread mmThread(&master_machine::master_status_send, mm);
    mm->inject_thread(mmThread);
    std::shared_ptr<master_machine> mm_ptr(mm);
    return mm_ptr;
}
