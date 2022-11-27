// socket functions

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
#define BNP_CONST_LEN sizeof(BNP_CONST)

// a public or private key
typedef int Key;

typedef struct {
  int fd; // the socket descriptor
  Key convKey; // the individualized key, if in a stream
  Key curTarget; // the current target for stream packets, if any
  uint8_t streamPacketsSent; // the amount of packets sent in the current stream
} bcn_socket_t;

enum PacketType
{
  CONTENT,
  SETUP,
  STREAM
};

struct bcn_packet_header {
  char constant[BNP_CONST_LEN]; // the constant
  enum PacketType type; // the packet's type
  uint8_t order; // unused unless packet is of type STREAM; if it is, then the amount of STREAM packets sent before this one
  uint32_t content_length; // the length of the content, in bytes
};

// creates and returns a socket
bcn_socket_t create_socket();

// destroys a socket
int destroy_socket(bcn_socket_t* bcn_socket);

// sends a content packet to a specified target
int send_content(bcn_socket_t *bcn_socket, char* buffer, int32_t bufSize, Key target, struct sockaddr addrChunk);

// recieves a content packet
int recv_content(bcn_socket_t *bcn_socket, char* buffer, int32_t bufsize);

// begins a stream with a specified target
int connect_to(bcn_socket_t *bcn_socket, Key target);

// accepts a connection
Key accept_from(bcn_socket_t* bcn_socket);

// sends a stream packet to a target
int send_stream(bcn_socket_t *bcn_socket, char* buffer, int32_t bufsize);

// recieves a stream packet
int recv_stream(bcn_socket_t *bcn_socket, char* buffer, int32_t bufsize);

#endif