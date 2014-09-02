/*
 * signal_handler.cpp
 *
 *  Created on: Sep 2, 2014
 *      Author: evangileon
 */

#include <signal.h>

#include "signal_handler.h"

struct sigaction old_act;

static void sig_term_handler(int signum) {

    //if(NULL == old_handler) {
    //    printf("Unexpected exception\n");
    //    exit(-1);
    //}

    //(*old_handler)(signum);

    unlink(config::instance().get_socket_failover_path().c_str());
    exit(signum + 128);
}

static void sig_int_handler(int signum) {

    unlink(config::instance().get_socket_failover_path().c_str());
    exit(signum + 128);
}

void setup_signal_handler() {
	struct sigaction term_act;
    struct sigaction int_act;

    memset(&term_act, 0, sizeof(term_act));
    term_act.sa_handler = &sig_term_handler;

    if(sigaction(SIGTERM, nullptr, &old_act) == -1) {
        perror("Can not get old handler of term signal");
        exit(-1);
    }

    if(sigaction(SIGTERM, &term_act, nullptr) == -1) {
        perror("Can not set new handler of term signal");
        exit(-1);
    }

    memset(&int_act, 0, sizeof(int_act));
    int_act.sa_handler = &sig_int_handler;
    if(sigaction(SIGINT, &int_act, nullptr) == -1) {
        perror("Can not set int signal");
        exit(-1);
    }
}
