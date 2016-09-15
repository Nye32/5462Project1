//__________________________
//| Andrew Lee				|
//| CSE5462	Lab 03			|
//| 09-14-16	srbaclib.c	|
//|_________________________|
// srbac Library

#include <sys/types.h>
#include <sys/socket.h>
#include <stddef.h>
#include "srbaclib.h"
#include <arpa/inet.h>

// SEND
ssize_t SEND(int sockfd, const void *buffer, size_t len, int flags) {
	return sendto(sockfd, buffer, len, flags, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
}

// RECV
ssize_t RECV(int sockfd, void *buf, size_t len, int flags) {
	return recvfrom(sockfd, buf, len, flags, &return_addr, &return_addrlen);
}

// BIND
int BIND(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	dest_addr = *addr;
	return bind(sockfd, (struct sockaddr *)addr, addrlen);
}

// ACCEPT
void ACCEPT(void) {

}

// CONNECT
void CONNECT(void) {

}

// SOCKET
int SOCKET(int family, int type, int protocol) {
	return socket(family, type, protocol);
}
