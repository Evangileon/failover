
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include "config.h"
#include "util.h"
#include "net_util.h"
#include "statusCheck.h"

using namespace std;

int needStatusSend = 0;
string whatTosend;

int status_receive_loop(int rfd) {
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
        if(ret != -1) {
            break;
        }
        if(ret > 16) {
        	cout << "I am hacked" << endl;
        	ret = -1;
        	break;
        }

        buffer[ret] = '\0';

        cout << "master is " << buffer << endl;
        sleep(1);
    }

    return ret;
}

void status_receive() {
    int sockfd;
    int ret;
    int status = 0;
    fd_set rfds;

    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    cli_len = sizeof(cli_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ((sockfd) < 0) {
        perror("ERROR opening socket");
        ERROR("%d\n", __LINE__);
    }

    int __jun_net_optval = 1;
    if(setsockopt((sockfd), SOL_SOCKET, SO_REUSEADDR, &__jun_net_optval, sizeof __jun_net_optval) < 0) {
        perror("errno on setsockopt");         
        ERROR("%d\n", __LINE__);    
    }  

    struct sockaddr_in __jun_net_serv_addr;
    bzero((void *) &__jun_net_serv_addr, sizeof(__jun_net_serv_addr));
    __jun_net_serv_addr.sin_family = AF_INET;
    __jun_net_serv_addr.sin_addr.s_addr = INADDR_ANY;
    __jun_net_serv_addr.sin_port = htons((STATUS_PORT));
    if (bind((sockfd), (struct sockaddr *) &__jun_net_serv_addr,sizeof(__jun_net_serv_addr)) < 0) {
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
            cout << "Not receive status from other\n";
        } else {
            // nothing to do
            cout << "status receive loop\n";
            int cfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
            if(cfd < 0) {
                perror("It shouldn't be error");
            }
            needStatusSend = 1;
           	int masterStatus = status_receive_loop(cfd);
            close(cfd);
        }
heart_beat_accept_fail:
        if(status < 0) {
        }
    }
}


string& checkResultStr() {
	
	int counter = system("ProcessChecker.sh");       
	if(counter!=0) { //application is down or not fully functional
		cout << "Entering process down section" << endl;
		        //system("/sbin/ifdown eth2"); // we bring down the interface and the asterisk service on the system
		        //system("/etc/init.d/asterisk stop");        
		whatTosend = "down";
		        // send the message no action from my side       
		        //asFailure=1;
				//isMaster=0;                   
		        //sleep(SLEEP_DUR);
	} else {
		cout << "Everything is fine" << endl;
		whatTosend="ok";
	}

	return whatTosend;
}

int master_status_send_loop(int wfd) {
    fd_set wfds; 
    int ret = -1;
    char buffer[16];

    while(1) {
        sleep(1);
        if(!needStatusSend) {
            break;
        }

        string whatToSend = checkResultStr();
		whatTosend.copy(buffer, whatToSend.length( ));
		buffer[whatTosend.length()] = '\0';

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
        cout << "status sent\n";
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
        
        cout << "status send loop\n";
        master_status_send_loop(sockfd);
        //cout << "cease sending\n";
        close(sockfd);
    }
}