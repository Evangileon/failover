#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <iostream>
#include <string>
#include <thread>

#include "config.h"
#include "util.h"
#include "net_util.h"
#include "standby_machine.h"
#include "machine_interaction.h"
#include "status_check.h"
#include "thread_util.h"
#include "async_handle_asterisk.h"
#include "checker.h"

standby_machine::standby_machine() {
	terminationFlag = 0;
	retval = 0;
}
standby_machine::~standby_machine() {
}

int standby_machine::status_receive_loop(int rfd) {
	fd_set rfds;
	int ret = 0;
	char buffer[SEND_BUFFER_SIZE];
	unsigned int timeout = config::instance().get_status_receive_loop_timeout();

	while (1) {

		if (terminationFlag) {
			std::cout << "standby termination flag = " << terminationFlag
					<< std::endl;
			if (MASTER_FAIL == terminationFlag) {
				ret = MASTER_FAIL;
				goto receive_loop_done;
			}
		}

		int check = checker::get_status();
		if(check != CHECK_ASTERISK_SHUT_DOWN) {
			async_handle_asterisk::stop();
		}

		ret = select_with_timeout(rfd, &rfds, timeout);
		if (ret <= 0) {
			ret = MASTER_FAIL;
			goto receive_loop_done;
		}
		if (!FD_ISSET(rfd, &rfds)) {
			ret = MASTER_FAIL;
			goto receive_loop_done;
		}

		ret = recv(rfd, buffer, SEND_BUFFER_SIZE, 0);
		if (ret <= 0) {
			ret = MASTER_FAIL;
			goto receive_loop_done;
		}
		if (ret > SEND_BUFFER_SIZE) {
			std::cout << "I am hacked" << std::endl;
			ret = -1;
			goto receive_loop_done;
		}

		buffer[ret] = '\0';

		std::cout << "master is " << buffer << std::endl;
		if (strcmp(buffer, "down") == 0) {
			ret = MASTER_ASTERISK_STOP;
			terminationFlag = MASTER_ASTERISK_STOP;
			goto receive_loop_done;
		}
		//sleep(1);
	}

receive_loop_done:
	std::cout << "End status receive loop" << std::endl;
	return ret;
}

void standby_machine::status_receive() {
	int sockfd;
	int ret;
	//int status = 0;
	int thread_exit_val;
	unsigned int timeout = config::instance().get_status_receive_timeout();
	fd_set rfds;

	struct sockaddr_in cli_addr;
	socklen_t cli_len;
	cli_len = sizeof(cli_addr);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if ((sockfd) < 0) {
		perror("ERROR opening socket");
		ERROR("%d\n", __LINE__);
	}

	// ensure the fd will be closed while calling pthread_cancel
	pthread_cleanup_push(&cleanup_handler, (void *)((long)sockfd));

		int net_optval = 1;
		if (setsockopt((sockfd), SOL_SOCKET, SO_REUSEADDR, &net_optval,
				sizeof net_optval) < 0) {
			perror("errno on setsockopt");
			ERROR("%s:%d\n", __FILE__, __LINE__);
		}

		if (config::instance().is_status_direct_link()) {
			enable_direct_link(sockfd);
		}

		if (get_tcp_connection_ready_socket(sockfd,
				config::instance().get_ip_standby_status_sender().c_str(),
				config::instance().get_port_status_receiver(), MAX_CONN_COUNT)
				< 0) {
			perror("...");
			ERROR("%s:%d\n", __FILE__, __LINE__);
		}

		while (1) {
			//sockfd = get_any_tcp_connection_ready_socket(sockfd, config::instance().get_port_heartbeat_recv, MAX_CONN_COUNT);
			if (sockfd < 0) {
				ERROR("%s, %d\n", __FILE__, __LINE__);
			}
			async_handle_asterisk::stop();

			ret = select_with_timeout(sockfd, &rfds, timeout);
			if (ret < 0) {
				perror("after select");
				ERROR("%s, %d\n", __FILE__, __LINE__);
			} else if (ret == 0) {
				// timeout
				// the other one is dead
				//status = -1;
				needStatusSend = 0;
				std::cout << "Not receive status from master\n";
				thread_exit_val = MASTER_ASTERISK_STOP;
				break;
			} else {
				// nothing to do
				std::cout << "status receive loop\n";
				int cfd = accept(sockfd, (struct sockaddr *) &cli_addr,
						&cli_len);
				if (cfd < 0) {
					perror("It shouldn't be error");
				}

				// ensure the fd will be closed while calling pthread_cancel
				pthread_cleanup_push(&cleanup_handler, (void *)((long)cfd));

					needStatusSend = 1;
					int masterStatus = status_receive_loop(cfd);
					if (masterStatus == MASTER_ASTERISK_STOP
							|| masterStatus == MASTER_FAIL) {
						close(cfd);
						thread_exit_val = MASTER_ASTERISK_STOP;
						break;
					}

				pthread_cleanup_pop(1);
				close(cfd);
			}
		}

		pthread_cleanup_pop(1);
	close(sockfd);
	std::cout << "status receive thread end" << std::endl;
	//*((int *)exit_val) = thread_exit_val;
	retval = thread_exit_val;
	//return NULL;
}

void standby_machine::inject_thread(std::thread &t) {
	standby_thread.swap(t);
}

void standby_machine::update(int flag) {

	switch (flag) {
	case THE_OTHER_IS_DEAD:
		terminationFlag = MASTER_FAIL;
		break;
	case THE_OTHER_IS_ALIVE:
		terminationFlag = MASTER_ALIVE;
		break;
	default:
		terminationFlag = 0;
	}
}

std::shared_ptr<standby_machine> init_standby_machine() {
	standby_machine *standbyMachine = new standby_machine();
	std::thread smThread(&standby_machine::status_receive, standbyMachine);
	standbyMachine->inject_thread(smThread);
	std::shared_ptr<standby_machine> mm_ptr(standbyMachine);
	return mm_ptr;
}
