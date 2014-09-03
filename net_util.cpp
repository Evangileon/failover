#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include "net_util.h"
#include "util.h"

int select_write_with_timeout(int sockfdq, fd_set *wfds, int time_sec) {

	FD_ZERO(wfds);
	FD_SET(sockfdq, wfds);

	struct timeval timeout;
	timeout.tv_sec = time_sec;  // It matter on the interval of heartbeat
	timeout.tv_usec = 0;

	int iResult = select(sockfdq + 1, (fd_set *) 0, wfds, (fd_set *) 0,
			&timeout);
	return iResult;
}

int select_with_timeout(int sockfdq, fd_set *rfds, int time_sec) {
	FD_ZERO(rfds);
	FD_SET(sockfdq, rfds);

	struct timeval timeout;
	timeout.tv_sec = time_sec;  // It matter on the interval of heartbeat
	timeout.tv_usec = 0;

	int iResult = select(sockfdq + 1, rfds, (fd_set *) 0, (fd_set *) 0,
			&timeout);
	return iResult;
}

static int do_get_connection_ready(int sockfd, int domain, int protocal,
		in_addr_t addr, in_port_t port, int count) {
	int sock;
	if (sockfd != -1) {
		sock = sockfd;
	} else {
		sock = socket(domain, protocal, 0);
		if ((sock) < 0) {
			perror("ERROR opening socket");
			ERROR("%d\n", __LINE__);
		}
	}

	int net_optval = 1;
	if (setsockopt((sock), SOL_SOCKET, SO_REUSEADDR, &net_optval,
			sizeof net_optval) < 0) {
		perror("errno on setsockopt");
		CUR_ERR()
		;
	}

	struct sockaddr_in net_serv_addr;
	bzero((void *) &net_serv_addr, sizeof(net_serv_addr));
	net_serv_addr.sin_family = domain;
	net_serv_addr.sin_addr.s_addr = addr;
	net_serv_addr.sin_port = htons(port);
	if (bind((sock), (struct sockaddr *) &net_serv_addr, sizeof(net_serv_addr))
			< 0) {
		perror("Error on bind");
		char addr_err[20];
		inet_ntop(AF_INET, (void *) &(net_serv_addr.sin_addr), addr_err,
				sizeof(addr_err));
		printf("Requested address: %s:%d\n", addr_err, port);
		CUR_ERR()
		;
	}
	if (listen((sock), (count)) < 0) {
		perror("Error on listen");
		CUR_ERR()
		;
	}
	return sock;
}

int get_connection_ready(int sockfd, int domain, int protocal, const char *addr,
		in_port_t port, int count) {
	return do_get_connection_ready(sockfd, domain, protocal, inet_addr(addr),
			(in_port_t) port, count);
}

int get_any_connection_ready(int sockfd, int domain, int protocal, int port,
		int count) {
	return do_get_connection_ready(sockfd, domain, protocal, INADDR_ANY,
			htons(port), count);
}

int get_tcp_connection_ready_socket(int socket, const char *addr, int port,
		int count) {
	return get_connection_ready(socket, AF_INET, SOCK_STREAM, addr, port, count);
}

int get_any_tcp_connection_ready_socket(int socket, int port, int count) {
	return get_any_connection_ready(socket, AF_INET, SOCK_STREAM, port, count);
}

int get_tcp_connection_ready(const char *addr, int port, int count) {
	return get_connection_ready(-1, AF_INET, SOCK_STREAM, addr, port, count);
}

int get_any_tcp_connection_ready(int port, int count) {
	return get_any_connection_ready(-1, AF_INET, SOCK_STREAM, port, count);
}

int connect_nonblock_raw(struct sockaddr_in *sa, int sock, unsigned timeout) {
	int flags = 0, error = 0, ret = 0;
	fd_set rset, wset;
	socklen_t len = sizeof(error);
	struct timeval ts;

	ts.tv_sec = timeout;

	FD_ZERO(&rset);
	FD_SET(sock, &rset);
	FD_ZERO(&wset);
	FD_SET(sock, &wset);

	if ((flags = fcntl(sock, F_GETFL, 0)) < 0) {
		CUR_INFO();
		return -1;
	}

	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
		CUR_INFO();
		return -1;
	}

	if ((ret = connect(sock, (struct sockaddr *) sa, 16)) < 0) {
		if (errno != EINPROGRESS) {
			CUR_INFO();
			return -1;
		}
	}

	if (ret == 0)    //then connect succeeded right away
		goto nonblock_done;

	if ((ret = select(sock + 1, &rset, &wset, NULL, (timeout) ? &ts : NULL))
			< 0) {
		CUR_INFO();
		return -1;
	}
	if (ret == 0) {   //we had a timeout
		errno = ETIMEDOUT;
		CUR_INFO();
		return -1;
	}

	if (FD_ISSET(sock, &rset) || FD_ISSET(sock, &wset)) {
		if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
			CUR_INFO();
			return -1;
		}
	} else {
		CUR_INFO();
		return -1;
	}

	if (error) {  //check if we had a socket error
		errno = error;
		perror("error: ");
		CUR_INFO();
		return -1;
	}

	nonblock_done: if (fcntl(sock, F_SETFL, flags) < 0) {
		CUR_INFO()
		;
		return -1;
	}

	return 0;
}

int connect_nonblock(const char *addr, int port, int sockfd,
		unsigned timeout_sec) {
	struct sockaddr_in receiver_addr;
	bzero((void *) &receiver_addr, sizeof(receiver_addr));
	receiver_addr.sin_family = AF_INET;
	receiver_addr.sin_addr.s_addr = inet_addr(addr);
	receiver_addr.sin_port = htons(port);

	return connect_nonblock_raw(&receiver_addr, sockfd, timeout_sec);
}

int sender_bind(const char *addr, int port, int sockfd) {
	struct sockaddr_in sender_addr;
	bzero((void *) &sender_addr, sizeof(sender_addr));
	sender_addr.sin_family = AF_INET;
	if (addr == NULL) {
		sender_addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		sender_addr.sin_addr.s_addr = inet_addr(addr);
	}
	sender_addr.sin_port = htons((in_port_t)port);

	int net_optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &net_optval,
			sizeof net_optval) < 0) {
		perror("errno on setsockopt");
		CUR_ERR();
	}

	if (bind(sockfd, (sockaddr *) &sender_addr, sizeof(sender_addr)) < 0) {
		perror("sender bind:");
		printf("Requested address: %s:%d\n", addr, port);
		CUR_ERR();
	}

	return 0;
}

int enable_direct_link(int sockfd) {
	int net_optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_DONTROUTE, &net_optval,
			sizeof net_optval) < 0) {
		perror("errno on setsockopt");
		CUR_ERR()
		;
	}
	return -1;
}
