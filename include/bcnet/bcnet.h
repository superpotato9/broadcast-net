// socket functions

#include <stdint.h>

#include "bcnet/auth.h"

#ifndef BRODCAST_NET_BCNET_H
#define BRODCAST_NET_BCNET_H

struct sockaddr_bcn {
  union {
    struct {
      uint8_t a;
      uint8_t b;
    };
    uint16_t full;
  } range;
  uint16_t port;
  bcn_key_t key;
};

typedef struct {
  int fd; // the socket descriptor
  bcn_key_t convKey; // the individualized key, if in a stream
  struct sockaddr_bcn curTarget; // the current target for stream packets, if any
  uint8_t streamPacketsSent; // the amount of packets sent in the current stream
} bcn_socket_t;

// creates and returns a socket
bcn_socket_t create_socket();

// destroys a socket
int destroy_socket(bcn_socket_t* bcn_socket);

// sends a content packet to a specified target
int send_content(bcn_socket_t *bcn_socket, char* buffer, int32_t bufSize, struct sockaddr_bcn target);

// recieves a content packet
int recv_content(bcn_socket_t *bcn_socket, char* buffer, int32_t bufsize, struct sockaddr_bcn fromaddr);

// begins a stream with a specified target
int connect_to(bcn_socket_t *bcn_socket, struct sockaddr_bcn target);

// accepts a connection
int accept_from(bcn_socket_t* bcn_socket);

// sends a stream packet to a target
int send_stream(bcn_socket_t *bcn_socket, char* buffer, int32_t bufsize);

// recieves a stream packet
int recv_stream(bcn_socket_t *bcn_socket, char* buffer, int32_t bufsize);

#endif