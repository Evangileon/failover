#include <fstream>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include <memory>

#include "util.h"
#include "net_util.h"
#include "heartbeat.h"
#include "master_machine.h"
#include "standby_machine.h"
#include "machine_interaction.h"
#include "config.h"


int create_socket() {

     int sfd = -1;
     struct sockaddr_un addr;
                               
     sfd = socket(AF_UNIX, SOCK_STREAM, 0);
     if (sfd == -1) {
         perror("socket");
         exit(-1);
     }

     int reuse = 1;
     if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
         perror("Can NOT set address to reuseable");
         exit(-1);
     }

     memset(&addr, 0, sizeof(struct sockaddr_un));
     /* Clear structure */
     addr.sun_family = AF_UNIX;
     strncpy(addr.sun_path, config::instance().get_socket_failover_path().c_str(), sizeof(addr.sun_path) - 1);

     if ((bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un))) < 0) {
         if(errno == EADDRINUSE) {
             printf("Failover is already in running\n");
         }
         perror("Can not bind on unix socket");
         exit(-1);
     }
    
     std::ofstream pidFile(config::instance().get_pid_failover_path().c_str());
     pidFile << getpid();
     pidFile.close();

     return sfd;
}

static struct sigaction old_act;
typedef void (*custom_sa_handler)(int);

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
    
    if(sigaction(SIGTERM, NULL, &old_act) == -1) {
        perror("Can not get old handler of term signal");
        exit(-1);
    }
    
    if(sigaction(SIGTERM, &term_act, NULL) == -1) {
        perror("Can not set new handler of term signal");
        exit(-1);
    }
    
    memset(&int_act, 0, sizeof(int_act));
    int_act.sa_handler = &sig_int_handler;
    if(sigaction(SIGINT, &int_act, NULL) == -1) {
        perror("Can not set int signal");
        exit(-1);
    }
}




pthread_mutex_t masterMutex;

int main(int argc, char const *argv[]) {
	int test = 0;
	if(1 < argc) {
        if(0 == strcmp(argv[1], "-x")) {
            test = 1;
        }
    }

    if(test) {
        std::cout << "This is a test\n";
    }

    config::instance().parse();
    
    setup_signal_handler();

    struct master_status_mtx mas_sta;

    if (config::instance().is_this_is_master()) {
        init_as_master(&mas_sta);
    } else {
        init_as_standby(&mas_sta);
    }

    std::shared_ptr<heartbeat> hb = init_heartbeat();

    int machine_ret = 0;

    // Infinite loop
    while(1) {
        // first check whether a master is alive

        if(is_this_master(&mas_sta)) {
            // first check whether a master is alive
            if(other_is_master()) {
                yield_master(&mas_sta);
                continue;
            }

            std::cout << "This is master" << std::endl;
            std::shared_ptr<master_machine> mm = init_master_machine();
            // add machine instance to heartbeat observer
            // It is an event-driven mechanism
            hb->attach_observer(std::dynamic_pointer_cast<observer>(mm));
            hb->attach_observer(std::dynamic_pointer_cast<observer>(config::instance().shared()));

            mm->join();

            machine_ret = mm->get_machine_retval();
            if(machine_ret == MASTER_ASTERISK_STOP) {
                yield_master(&mas_sta);
            }

        } else {
            std::cout << "This is standby" << std::endl;
            std::shared_ptr<standby_machine> sm = init_standby_machine();
            hb->attach_observer(std::dynamic_pointer_cast<observer>(sm));
            hb->attach_observer(std::dynamic_pointer_cast<observer>(config::instance().shared()));

            sm->join();
            
            machine_ret = sm->get_machine_retval();
            if(machine_ret == MASTER_ASTERISK_STOP) {
                seize_master(&mas_sta);
            }		
        }

        // Clear observers
        hb->reset_observer();
    }

	#if 0
	
	#endif
    
    std::cout << "In the end\n";
    //heartbeatSendThread.join();
    //heartbeatReceiveThread.join();
    std::cout << "After join\n";
    unlink(config::instance().get_socket_failover_path().c_str());
    
    //close(sfd);

	return 0;
}
