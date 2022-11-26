#include "bcnet/bcnet.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>

create_socket() {
    return socket(AF_INET, SOCK_DGRAM, 0);
}

int destroy_socket(int sockfd)
{
  return close(sockfd);
}
