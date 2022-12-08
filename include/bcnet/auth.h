//
// Created by nathan on 11/25/22.
//

#ifndef BROADCAST_NET_AUTH_H
#define BROADCAST_NET_AUTH_H

//int RSA_public_encrypt(int flen, unsigned char *from,
//                       unsigned char *to, RSA *rsa, int padding);
char* rsa_encrypt(char *pub_key, char *msg); // takes public key and  message and returns the rsa encryted message
char* rsa_decrypt(char *priv_key, char *msg); // take priv key and encrypted message and returns rsa decrypted message

char* encrypt(char *key, char *msg); // encrypts message using TBD protocol
char* decrypt(char *priv_key, char *msg); // decrypts message using TBD protocol 
#endif //BROADCAST_NET_AUTH_H

