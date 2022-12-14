
# Broadcast Network protocol
abbrev to BNP



A library for Broadcast Network Protocol (BNP) sockets.




## BNP Basics

BNP is a network protocol based on UDP that aims for 100% anonymity by removing the need to know the IP address of your target.  A packet header consists of:
 - A constant to test if the packet is properly decrypted using your private key (size TBD)
 - The type of packet (content, setup, or stream) (1 byte)
 	-  Content packets are encrypted using the target's known public key and contain data.  These are one-off messages, used when an extended conversation is not expected.
 	-  Setup packets are encrypted using either the target's known public key or their individualized public key and contain an individualized public key for the target system to use for stream packets to the sender.  These are used to set up a stream between two systems.
 	- Stream packets are encrypted using the target's individualized public key and contain data.  These are used instead of content packets when a stream has already been set up between two systems.
 - If the packet is a stream packet, the number of stream packets sent before this one is sent as well, to preserve order the number of stream packets sent before is sent; otherwise this is unused (1 byte)
 - The length of the data (4 bytes)

The entire packet is encrypted using the target's public key.  Public-private key pairs are unique to the network adapter and are generated using its MAC address.

A typical conversation between two systems using a BNP stream looks like this:
 1. System A generates an individualized keypair for the conversation and sends the generated public key to System B
 2. System B receives the setup packet with the public key and generates its own individualized keypair, sending that public key back to System A
 3. System A will now send stream packets to System B encrypted using the public key they received from System B, who will do likewise with System A's public key
 4. If either system receives a content or setup packet from the other system, the conversation is assumed to have ended and the individualized keypairs are discarded



## Features

- works with standard TCP/IP 
- outside observers are unable to see who is talking to who
- sender and receiver unable to know each others location
- SSL encryption
- the ability to do one-off packets and packet streams
- fully decentralized




## License

[MIT](https://choosealicense.com/licenses/mit/)


## Maintainers

 - [superpotato9](https://github.com/superpotato9)
 - [FreehandBlock51](https://github.com/FreehandBlock51)
 
