
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>

#include "config.h"
#include "util.h"
#include "net_util.h"
#include "machine_interaction.h"

static int init_master_status(struct master_status_mtx *mas_sta) {
    if (pthread_mutex_init(&mas_sta->mutex, NULL)) {
        perror("yield master");
        CUR_ERR();
    }
    return 0;
}

int init_machine_as(struct master_status_mtx *mas_sta, int isMaster) {
    init_master_status(mas_sta);
    mas_sta->isMaster = isMaster;
    return isMaster;
}

int is_this_master(struct master_status_mtx *mas_sta) {
    int isMaster;
    if (pthread_mutex_lock(&mas_sta->mutex)) {
        perror("yield master");
        CUR_ERR();
    }

    isMaster = mas_sta->isMaster;

    if (pthread_mutex_unlock(&mas_sta->mutex)) {
        perror("yield master");
        CUR_ERR();
    }

    return isMaster;
}

int yield_master(struct master_status_mtx *mas_sta) {
    if (pthread_mutex_lock(&mas_sta->mutex)) {
        perror("yield master");
        CUR_ERR();
    }

    mas_sta->isMaster = 0;

    if (pthread_mutex_unlock(&mas_sta->mutex)) {
        perror("yield master");
        CUR_ERR();
    }
    return 0; // current status
}

thread_terminator::thread_terminator() { thread_id = 0; }
thread_terminator::~thread_terminator() {}

thread_terminator::thread_terminator(pthread_t id) {
    set_thread_id(id);
}

int thread_terminator::terminate() {
    int ret;
    if ((ret = pthread_cancel(this->thread_id)) != 0) {
        perror("thread cancel");
    }
    return ret;
}

void thread_terminator::set_thread_id(pthread_t id) {
    this->thread_id = id;
}

int other_is_master() {
    int sockfd;
    int ret;
    int status = 0;
    fd_set rfds;

    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    cli_len = sizeof(cli_addr);

    sockfd = get_any_tcp_connection_ready(config::instance().get_port_status_receiver(), MAX_CONN_COUNT);
    if (sockfd < 0) {
        ERROR("%s, %d\n", __FILE__, __LINE__);
    }

    ret = select_with_timeout(sockfd, &rfds, 2);
    if (ret < 0) {
        perror("after select");
        ERROR("%s, %d\n", __FILE__, __LINE__);
    } else if (ret == 0) {
        // timeout
        // the other one is dead
        status = -1;
        std::cout << "Not exist a master\n";
        return 0;
    } else {
        // nothing to do
        int cfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
        if (cfd < 0) {
            perror("It shouldn't be error");
        }

        std:: cout << "alreay exist a master, so this is switched to standby\n";
        close(cfd);
        return 1;
    }
    return status;
}

int seize_master(struct master_status_mtx *mas_sta) {
    if (pthread_mutex_lock(&mas_sta->mutex)) {
        perror("yield master");
        CUR_ERR();
    }

    mas_sta->isMaster = 1;

    if (pthread_mutex_unlock(&mas_sta->mutex)) {
        perror("yield master");
        CUR_ERR();
    }
    return 1; // current status
}
