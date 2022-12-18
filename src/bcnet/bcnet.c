// platform-independent implementations of functions from 'bcnet.h'
#include "bcnet/bcnet.h"

#if defined(_WIN32) || defined(__WIN32__)
# include <WinSock2.h>
#else
# include <unistd.h>
# include <sys/socket.h>
#include <netdb.h>
# define closesocket close
#endif

#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>


/* Internal Definitions
 * These are necessary for the implementations of functions,
 * but are not used by the end user.  We put them here to
 * keep bcnet.h small.
 */
// the constant
#define BCN_CONST "This is a test constant"
#define BCN_CONST_LEN sizeof(BCN_CONST)

// the type of a BNP packet
enum PacketType
{
  CONTENT,
  SETUP,
  STREAM
};

// the header of a BNP packet
struct bcn_packet_header {
  char constant[BCN_CONST_LEN]; // the constant
  enum PacketType type; // the packet's type
  uint8_t order; // unused unless packet is of type STREAM; if it is, then the amount of STREAM packets sent before this one
  uint32_t content_length; // the length of the content, in bytes
};

// populates a BNP packet
int bcn_populate_packet(char *packet, size_t packetsize, enum PacketType type, uint8_t order, uint32_t content_length, char* content) {
  if (packetsize < sizeof(struct bcn_packet_header) + (content_length * 8)) { // buffer for packet is too small
    errno = EINVAL;
    return -1;
  }

  struct bcn_packet_header *header = (struct bcn_packet_header*)packet;
  *header = (struct bcn_packet_header){
    .constant = BCN_CONST,
    .type = type,
    .order = order,
    .content_length = content_length
  };
  char *body = packet + sizeof(struct bcn_packet_header);
  memcpy(body, content, content_length * 8); // multiply content_length by 8 becuase it specifies length in bytes, but we need length in bits
  return 0;
}

/* Function Implementations
 * These are implementations of functions defined in bcnet.h
 */
bcn_socket_t create_socket() {
  bcn_socket_t bcn_socket = (bcn_socket_t){ 0 };
  bcn_socket.fd = socket(AF_INET, SOCK_DGRAM, 0);
  // convKey & curTarget are arrays, so they're already initilized
  bcn_socket.streamPacketsSent = 0;
  return bcn_socket;
}

int destroy_socket(bcn_socket_t* bcn_socket) {
  int result;
  if ((result = closesocket(bcn_socket->fd)) != 0) {
    return result;
  }
  memset(bcn_socket, 0, sizeof(bcn_socket_t));
  return 0;
}

int send_content(bcn_socket_t* bcn_socket, char* buffer, int32_t bufSize, struct sockaddr_bcn target) {
  if (bcn_socket->streamPacketsSent > 0) { // stream sockets sending content packets will lose their connection to the stream
    bcn_socket->streamPacketsSent = 0;
    memset(bcn_socket->convKey, 0, sizeof(bcn_key_t));
    memset(&bcn_socket->curTarget, 0, sizeof(struct sockaddr_bcn));
  }

  // create the packet
  size_t packetSize = sizeof(struct bcn_packet_header) + bufSize;
  char *packet;
  char *encryptedpacket;
  if ((packet = malloc(packetSize)) == NULL || (encryptedpacket = malloc(packetSize)) == NULL) {
    free(packet);
    free(encryptedpacket); // free both in case one was allocated
    return -1; // errno set by malloc
  }
  if (!bcn_populate_packet(packet, packetSize, CONTENT, 0, bufSize / 8, buffer)) {
    free(packet);
    free(encryptedpacket);
    return -1; // errno set by bcn_populate_packet
  }

  // encrypt the packet
  if (!rsa_encrypt(target.key, packet, packetSize, encryptedpacket, packetSize)) {
    free(packet);
    free(encryptedpacket);
    return -1; // errno should be set by rsa_encrypt
  }

  // brodcast the packet
  struct sockaddr_in tempaddr = {
    .sin_family = AF_INET,
    .sin_addr = {
      .s_addr = target.range.full << 16
    },
    .sin_port = target.port,
    .sin_zero = {0,0,0,0,0,0,0,0}
  };
  for (uint16_t lastIpHalf = 0; lastIpHalf < UINT16_MAX; lastIpHalf++) {
    tempaddr.sin_addr.s_addr++;
    sendto(bcn_socket->fd, encryptedpacket, packetSize, 0, (struct sockaddr*)&tempaddr, sizeof(struct sockaddr));
  }

  free(packet);
  free(encryptedpacket);
  return 0;
}

int recv_content(bcn_socket_t* bcn_socket, char* buffer, int32_t bufsize, struct sockaddr_bcn fromaddr) {
  if (bcn_socket->streamPacketsSent > 0) { // stream sockets sending content packets will lose their connection to the stream
    bcn_socket->streamPacketsSent = 0;
    memset(bcn_socket->convKey, 0, sizeof(bcn_key_t));
    memset(&bcn_socket->curTarget, 0, sizeof(struct sockaddr_bcn));
  }

  int result = 0;

  // recieve the packet
  struct sockaddr_in tempaddr = (struct sockaddr_in){
    .sin_family = AF_INET,
    .sin_addr = {
      .s_addr = fromaddr.range.full << 16
    },
    .sin_port = fromaddr.port,
    .sin_zero = {0,0,0,0,0,0,0,0}
  };

  size_t packetsize = bufsize + sizeof(struct bcn_packet_header);
  char *encryptedpacket = malloc(packetsize);
  char *decryptedpacket = malloc(packetsize);
  if (encryptedpacket == NULL || decryptedpacket == NULL) {
    free(encryptedpacket);
    free(decryptedpacket); // free both, in case one of the allocations worked
    return -1; // errno set by malloc
  }
  socklen_t tempaddrlen = sizeof(tempaddr);

  for (uint16_t lastIpHalf = 0; lastIpHalf < UINT16_MAX; lastIpHalf++) {
    tempaddr.sin_addr.s_addr++;
    memset(encryptedpacket, 0, packetsize); // clear packet buffer to prevent corruption
    result = recvfrom(bcn_socket->fd, encryptedpacket, packetsize, 0, (struct sockaddr*)&tempaddr, &tempaddrlen);

    // check if the packet is ours
    if (result < sizeof(struct bcn_packet_header)) { // packet is smaller than our header size, and therefore can't be ours
      continue;
    }
    if (!(result = rsa_decrypt(fromaddr.key, encryptedpacket, packetsize, decryptedpacket, packetsize))) { // decryption error
      break; // cleanup is done after the loop, & errno is set by rsa_decrypt
    }
    struct bcn_packet_header *header = (struct bcn_packet_header*)decryptedpacket;
    if (header->constant != BCN_CONST || header->type != CONTENT) {
      memset(decryptedpacket, 0, packetsize); // clear decrypted buffer to prevent corruption
      continue;
    }
    if (header->content_length * 8 > bufsize) { // buffer is too small
      errno = EFBIG; // 'file is too big' (in our case, the file is a packet)
      result = -1;
      break; // cleanup is after the loop
    }

    // get the message content
    char* body = decryptedpacket + sizeof(struct bcn_packet_header);
    if (memccpy(buffer, body, 0, bufsize) != NULL) { // buffer doesn't end in 0 like a standard C string; if let unchecked, this could cause overflows
      errno = EMSGSIZE; // packet might be bigger than the buffer
      result = 1; // not an error, just a warning
    }
    break; // result will either be 0 or errno will be set.  Either way, cleanup is after the loop
  }
  
  // cleanup
  free(encryptedpacket);
  free(decryptedpacket);

  return result;
}

int connect_to(bcn_socket_t *bcn_socket, struct sockaddr_bcn target) {
  // this socket is now a stream socket, so we need to initilize the corresponding values
  memcpy(&bcn_socket->curTarget, &target, sizeof(struct sockaddr_bcn));
  create_key(bcn_socket->convKey);
  bcn_socket->streamPacketsSent = 0;

  // create the setup packet
  size_t packetSize = sizeof(struct bcn_packet_header) + sizeof(bcn_key_t);
  char *packet, *encryptedPacket;
  if ((packet = malloc(packetSize)) == NULL || (encryptedPacket = malloc(packetSize)) == NULL) { // malloc error
    free(packet);
    free(encryptedPacket); // free both buffers incase one malloc was successful
    return -1; // errno is set by malloc
  }
  if (!bcn_populate_packet(packet, packetSize, CONTENT, 0, sizeof(bcn_key_t) / 8, bcn_socket->convKey)) {
    free(packet);
    free(encryptedPacket);
    return -1; // errno set by bcn_populate_packet
  }

  // encrypt the packet
  if (!rsa_encrypt(target.key, packet, packetSize, encryptedPacket, packetSize)) {
    free(packet);
    free(encryptedPacket);
    return -1; // errno should be set by rsa_encrypt
  }

  // brodcast the packet
  struct sockaddr_in tempaddr = {
    .sin_family = AF_INET,
    .sin_addr = {
      .s_addr = target.range.full << 16
    },
    .sin_port = target.port,
    .sin_zero = {0,0,0,0,0,0,0,0}
  };
  for (uint16_t lastIpHalf = 0; lastIpHalf < UINT16_MAX; lastIpHalf++) {
    tempaddr.sin_addr.s_addr++;
    sendto(bcn_socket->fd, encryptedPacket, packetSize, 0, (struct sockaddr*)&tempaddr, sizeof(struct sockaddr));
  }

  // cleanup
  free(packet);
  free(encryptedPacket);

  return 0;
}

int accept_from(bcn_socket_t* bcn_socket) {

}