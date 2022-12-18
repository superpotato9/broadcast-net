//
// Created by nathan on 11/25/22.
//

#include <stddef.h>

#ifndef BROADCAST_NET_AUTH_H
#define BROADCAST_NET_AUTH_H

// the length of a key, in bytes
#define BCN_KEY_LEN 256

// a public or private key
typedef char bcn_key_t[BCN_KEY_LEN];

// assigns a value to a key (TODO improve performance)
#define bcn_assign_key(src, dest) memcpy(dest, src, sizeof(bcn_key_t))


// takes public key and  message and returns the rsa encryted message
int rsa_encrypt(bcn_key_t pub_key, char *msg, size_t msglen, char *encrypted, size_t encryptedlen);

// take private key and encrypted message and returns rsa decrypted message
int rsa_decrypt(bcn_key_t priv_key, char *msg, size_t msglen, char *decrypted, size_t decryptedlen);

// encrypts message using TBD protocol
int encrypt(bcn_key_t key, char *msg, size_t msglen, char *encrypted, size_t encryptedlen);

// decrypts message using TBD protocol 
int decrypt(bcn_key_t priv_key, char *msg, size_t msglen, char *decrypted, size_t decryptedlen);
#endif //BROADCAST_NET_AUTH_H

