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
    memset(bcn_socket->curTarget, 0, sizeof(bcn_key_t));
  }

  size_t packetSize = sizeof(struct bcn_packet_header) + bufSize;
  char *packet = malloc(packetSize);
  if (packet == NULL) {
    return -1; // errno set by malloc
  }

  // populate the header
  struct bcn_packet_header *header = (struct bcn_packet_header*)packet;
  *header = (struct bcn_packet_header){ .constant = BNP_CONST };
  header->content_length = bufSize;
  header->type = CONTENT;
  header->order = 0;

  // populate the body
  char* body = packet + sizeof(struct bcn_packet_header);
  if (!rsa_encrypt(target.key, buffer, bufSize, body, sizeof(body))) {
    free(packet);
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
    sendto(bcn_socket->fd, packet, packetSize, 0, (struct sockaddr*)&tempaddr, sizeof(struct sockaddr));
  }

  free(packet);
  return 0;
}

int recv_content(bcn_socket_t* bcn_socket, char* buffer, int32_t bufsize, struct sockaddr_bcn fromaddr) {
  if (bcn_socket->streamPacketsSent > 0) { // stream sockets sending content packets will lose their connection to the stream
    bcn_socket->streamPacketsSent = 0;
    memset(bcn_socket->convKey, 0, sizeof(bcn_key_t));
    memset(bcn_socket->curTarget, 0, sizeof(bcn_key_t));
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
    result = recvfrom(bcn_socket->fd, encryptedpacket, packetsize, 0, (struct sockaddr*)&tempaddr, &tempaddrlen); // TODO do we need to cast tempaddr to a struct sockaddr?

    // check if the packet is ours
    if (result < sizeof(struct bcn_packet_header)) { // packet is smaller than our header size, and therefore can't be ours
      continue;
    }
    if (!(result = rsa_decrypt(fromaddr.key, encryptedpacket, packetsize, decryptedpacket, packetsize))) { // decryption error
      break; // cleanup is done after the loop, & errno is set by rsa_decrypt
    }
    struct bcn_packet_header *header = (struct bcn_packet_header*)decryptedpacket;
    if (header->constant != BNP_CONST || header->type != CONTENT) {
      memset(decryptedpacket, 0, packetsize); // clear decrypted buffer to prevent corruption
      continue;
    }

    // get the message content
    char* body = decryptedpacket + sizeof(struct bcn_packet_header);
    if (memccpy(buffer, body, 0, bufsize) != NULL) { // buffer doesn't end in 0 like a standard C string; if let unchecked, this could cause overflows
      errno = EMSGSIZE; // packet might be bigger than the buffer
      result = 1; // not an error, just a warning
    }
    break; // result will either be 0 or errno will be set.  Either way, cleanup is after the loop
  }
  free(encryptedpacket);
  free(decryptedpacket);

  return result;
}
