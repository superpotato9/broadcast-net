#include "bcnet/bcnet.h"

#include <sys/socket.h>

create_socket() {
    return socket(AF_INET, SOCK_DGRAM, 0);
}