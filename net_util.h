#ifndef __NET_UTIL_H_JUN__
#define __NET_UTIL_H_JUN__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define create_connection(sock, ipaddr, port, count) do {  \
        (sock) = socket(AF_INET, SOCK_STREAM, 0);  \
        if ((sock) < 0) {                          \
            perror("ERROR opening socket");         \
        }                                           \
        int __jun_net_optval = 1;                   \
        if(setsockopt((sock), SOL_SOCKET, SO_REUSEADDR, &__jun_net_optval, sizeof __jun_net_optval) < 0) { \
            perror("errno on setsockopt");                                        \
        }                                           \
        struct sockaddr_in __jun_net_serv_addr;     \
        bzero((char *) &__jun_net_serv_addr, sizeof(serv_addr));  \
                                                                    \
        __jun_net_serv_addr.sin_family = AF_INET;                 \
        __jun_net_serv_addr.sin_addr.s_addr = (ipaddr);         \
        __jun_net_serv_addr.sin_port = htons((port));             \
        if (bind((sock), (struct sockaddr *) &__jun_net_serv_addr,sizeof(__jun_net_serv_addr)) < 0) { \
            perror("ERROR on binding, receiveMessage"); \
        }                                               \
        if(listen((sock),(count)) < 0) {                \
            perror("listen, receiveMessage");           \
        }                                               \
    } while(0)

extern int select_with_timeout(int sockfd, fd_set* rfds, int time_sec);
extern int select_write_with_timeout(int, fd_set*, int);
extern int get_tcp_connection_ready(const char* addr, int port, int count);
extern int get_any_connection_ready(int, int, int, int, int);
extern int get_any_tcp_connection_ready(int, int);
extern int get_tcp_connection_ready_socket(int socket, const char* addr, int port, int count);
extern int get_any_tcp_connection_ready_socket(int, int, int);
extern int connect_nonblock(struct sockaddr_in *, int, int);

#endif
