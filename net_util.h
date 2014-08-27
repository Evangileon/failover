#ifndef __NET_UTIL_H_JUN__
#define __NET_UTIL_H_JUN__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>



extern int select_with_timeout(int sockfd, fd_set* rfds, int time_sec);
extern int select_write_with_timeout(int, fd_set*, int);
extern int get_tcp_connection_ready(const char* addr, int port, int count);
extern int get_any_connection_ready(int, int, int, int, int);
extern int get_any_tcp_connection_ready(int, int);
extern int get_tcp_connection_ready_socket(int socket, const char* addr, int port, int count);
extern int get_any_tcp_connection_ready_socket(int, int, int);
extern int connect_nonblock_raw(struct sockaddr_in* sa, int sock, unsigned timeout);
extern int connect_nonblock(const char* addr, int port, int sockfd, unsigned timeout_sec);
extern int sender_bind(const char* addr, int port, int sockfd);

#endif
