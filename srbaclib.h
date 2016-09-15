//__________________________
//| Andrew Lee				|
//| CSE5462	Lab 03			|
//| 09-14-16	srbaclib.h	|
//|_________________________|
// Header file for srbac Library

#include <sys/types.h>
#include <sys/socket.h>

// Global Variables
struct sockaddr dest_addr;

// SEND
ssize_t SEND(int sockfd, const void *buffer, size_t len, int flags);

// RECV
ssize_t RECV(int sockfd, void *buf, size_t len, int flags);

// BIND
int BIND(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// ACCEPT
void ACCEPT(void);

// CONNECT
void CONNECT(void);

// SOCKET
int SOCKET(int family, int type, int protocol);