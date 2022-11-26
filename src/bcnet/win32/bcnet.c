#include "bcnet/bcnet.h"

#include <WinSock2.h>
#include <stdio.h>

create_socket() {
  return socket(AF_INET, SOCK_DGRAM, 0);
}

destroy_socket(int sockfd) {
  return closesocket(sockfd);
}
