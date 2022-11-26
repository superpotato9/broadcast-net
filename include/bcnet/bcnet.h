#if defined(_WIN32) || defined(__WIN32__)
# include <WinSock2.h>
#else
# include <sys/socket.h>
#endif

#include <stdint.h>

#ifndef BRODCAST_NET_BCNET_H
#define BRODCAST_NET_BCNET_H

// the constant
#define BNP_CONST "This is a test constant"

#define KEY int

// creates and returns a socket
int create_socket();

// destroys a socket
int destroy_socket(int sockfd);

// sends a content packet to a specified target
int send_content(int sockfd, char* buffer, int32_t bufsize, KEY target);

// recieves a content packet
int recv_content(int sockfd, char* buffer, int32_t bufsize);

// begins a stream with a specified target
int connect_to(int sockfd, KEY target);

// accepts a connection
KEY accept_from(int sockfd);

// sends a stream packet to a target
int send_stream(int sockfd, char* buffer, int32_t bufsize, KEY target);

// recieves a stream packet
int recv_stream(int sockfd, char* buffer, int32_t bufsize, KEY target);

#endif