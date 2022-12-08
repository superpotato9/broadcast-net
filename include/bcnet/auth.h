//
// Created by nathan on 11/25/22.
//

#include <stddef.h>

#ifndef BROADCAST_NET_AUTH_H
#define BROADCAST_NET_AUTH_H

// a public or private key
typedef char key_t[256];

// takes public key and  message and returns the rsa encryted message
int rsa_encrypt(key_t pub_key, char *msg, size_t msglen, char *encrypted, size_t encryptedlen);

// take private key and encrypted message and returns rsa decrypted message
int rsa_decrypt(key_t priv_key, char *msg, size_t msglen, char *decrypted, size_t decryptedlen);

// encrypts message using TBD protocol
int encrypt(key_t key, char *msg, size_t msglen, char *encrypted, size_t encryptedlen);

// decrypts message using TBD protocol 
int decrypt(key_t priv_key, char *msg, size_t msglen, char *decrypted, size_t decryptedlen);
#endif //BROADCAST_NET_AUTH_H

