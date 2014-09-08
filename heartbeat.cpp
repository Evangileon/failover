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
#include "thread_util.h"

heartbeat::heartbeat() {
	needSend = 1;
	status_recv_thread_id = -1;
	status_send_thread_id = -1;
}

heartbeat::~heartbeat() {

}

int heartbeat::heartbeat_receive_loop(int rfd) {
	fd_set rfds;
	int ret = -1;
	char buffer[16];
	unsigned timeout = config::instance().get_heartbeat_receive_loop_timeout();

	while (1) {
		ret = select_with_timeout(rfd, &rfds, timeout);
		if (ret <= 0) {
			break;
		}
		if (!FD_ISSET(rfd, &rfds)) {
			break;
		}

		ret = recv(rfd, buffer, 16, 0);
		if (ret != 16) {
			break;
		}
		notify_observers(THE_OTHER_IS_ALIVE);
		std::cout << "heartbeat recv succeed" << std::endl;
		sleep(1);
	}

	return ret;
}

int heartbeat::heartbeat_send_loop(int wfd) {
	fd_set wfds;
	int ret = 0;
	char buffer[16];
	unsigned interval = config::instance().get_heartbeat_send_interval();
	unsigned timeout = config::instance().get_heartbeat_send_loop_timeout();

	while (1) {
		sleep(interval);

		ret = select_write_with_timeout(wfd, &wfds, timeout);
		if (ret <= 0) {
			ret = -1;
			break;
		}
		if (!FD_ISSET(wfd, &wfds)) {
			ret = -1;
			break;
		}

		ret = send(wfd, buffer, 16, MSG_NOSIGNAL);
		//DEBUG("errno = %d\n", errno);
		if (ret != 16) {
			ret = -1;
			break;
		}
		std::cout << "heartbeat send succeed" << std::endl;
	}

	return ret;
}

void heartbeat::heartbeat_receive() {
	heartbeat_recv_thread_id = std::this_thread::get_id();

	int sockfd;
	int ret;
	int status = 0;
	unsigned timeout = config::instance().get_heartbeat_receive_timeout();
	fd_set rfds;

	struct sockaddr_in cli_addr;
	socklen_t cli_len;
	cli_len = sizeof(cli_addr);

	sockfd = get_tcp_connection_ready(
			config::instance().get_ip_heartbeat_sender().c_str(),
			config::instance().get_port_heartbeat_receiver(),
			MAX_CONN_COUNT);
	if ((sockfd) < 0) {
		perror("ERROR opening socket");
		ERROR("%d\n", __LINE__);
	}
	pthread_cleanup_push(&cleanup_handler, (void *)((long)sockfd));

	if (config::instance().is_heartbeat_direct_link()) {
		enable_direct_link(sockfd);
	}

	while (1) {

		ret = select_with_timeout(sockfd, &rfds, timeout);
		if (ret < 0) {
			perror("after select");
			ERROR("%s, %d\n", __FILE__, __LINE__);
		} else if (ret == 0) {
			// timeout
			// the other one is dead
			status = -1;
			needSend = 0;
			std::cout << "The other is dead" << std::endl;
			sleep(1);
			notify_observers(THE_OTHER_IS_DEAD);
		} else {
			// nothing to do
			std::cout << "go to heartbeat receive loop" << std::endl;
			int cfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
			if (cfd < 0) {
				perror("It shouldn't be error");
			}
			needSend = 1;
			notify_observers(THE_OTHER_IS_ALIVE);
			heartbeat_receive_loop(cfd);
			close(cfd);
		}
	}
	pthread_cleanup_pop(1);

	close(sockfd);
}

void heartbeat::heartbeat_send() {
	heartbeat_send_thread_id = std::this_thread::get_id();

	int sockfd;

	char receiver_addr[20];
	std::string receiver_addr_s =
			config::instance().get_ip_heartbeat_receiver();
	int receiver_port = config::instance().get_port_heartbeat_receiver();
	receiver_addr_s.copy(receiver_addr, receiver_addr_s.length());
	unsigned timeout = config::instance().get_connect_nonblock_timeout();

	std::cout << "heartbeat receiver addr: " << receiver_addr << ":"
			<< receiver_port << std::endl;

	//char sender_addr[20];
	//std::string sender_addr_s = config::instance().get_ip_heartbeat_sender();
	int sender_port = config::instance().get_port_heartbeat_sender();
	//sender_addr_s.copy(sender_addr, sender_addr_s.length());

	std::cout << "heartbeat sender bind to addr: "
			<< config::instance().get_ip_heartbeat_sender() << ":"
			<< sender_port << std::endl;

	pthread_cleanup_push(&cleanup_handler, (void *)((long)sockfd));
heartbeat_send_begin:
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		ERROR("%d\n", __LINE__);
	}

	if (config::instance().is_heartbeat_direct_link()) {
		enable_direct_link(sockfd);
	}

	if (sender_bind(NULL, sender_port, sockfd) < 0) {
		std::cout << "Can not bind" << std::endl;
	}

	while (1) {

		int ret = connect_nonblock(receiver_addr, receiver_port, sockfd,
				timeout);
		if (ret < 0) {
			if (errno == ECONNREFUSED) {
				std::cout << "connection refused" << std::endl;
				sleep(1);
				continue;
			}

			perror("heartbeat connect after nonblock");
			printf("Requested address: %s:%d\n", receiver_addr, receiver_port);
			sleep(1);
			close(sockfd);
			goto heartbeat_send_begin;
			CUR_INFO();
		}

		if (errno == ETIMEDOUT) {
			// time out
			std::cout << "heartbeat send timeout" << std::endl;
			continue;
		}

		std::cout << "go to send loop" << std::endl;
		int ret_loop = heartbeat_send_loop(sockfd);
		if (ret_loop < 0) {
			close(sockfd);
			goto heartbeat_send_begin;
		}

		std::cout << "cease sending" << std::endl;
	}
	pthread_cleanup_pop(1);
	close(sockfd);
}

void heartbeat::notify_observers(int flag) {
	for (std::vector<std::shared_ptr<observer> >::iterator iter = views.begin();
			iter != views.end(); ++iter) {
		(*iter)->update(flag);
	}
}

void heartbeat::start_heartbeat_send() {

}

void heartbeat::start_heartbeat_recv() {

}

std::shared_ptr<heartbeat> init_heartbeat() {
	heartbeat *hb = new heartbeat();
	std::thread hbRecv(&heartbeat::heartbeat_send, hb);
	hbRecv.detach();
	sleep(4);
	std::thread hbSend(&heartbeat::heartbeat_receive, hb);
	hbSend.detach();

	std::shared_ptr<heartbeat> hb_ptr(hb);
	return hb_ptr;
}
