//
// Created by nathan on 11/25/22.
//

#include <stddef.h>

#ifndef BROADCAST_NET_AUTH_H
#define BROADCAST_NET_AUTH_H

// a public or private key
typedef struct Key {
    // the key string
    char* keystr;
    // the length of the key string
    size_t keylen;
} key_t;

// takes public key and  message and returns the rsa encryted message
char* rsa_encrypt(key_t pub_key, char *msg, size_t msglen);

// take private key and encrypted message and returns rsa decrypted message
char* rsa_decrypt(key_t priv_key, char *msg, size_t msglen);

// encrypts message using TBD protocol
char* encrypt(key_t key, char *msg, size_t msglen);

// decrypts message using TBD protocol 
char* decrypt(key_t priv_key, char *msg, size_t msglen);
#endif //BROADCAST_NET_AUTH_H

