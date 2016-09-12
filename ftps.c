//_______________________
//| Andrew Lee			|
//| CSE5462	Lab 02		|
//| 09-05-16	ftps.c	|
//|_____________________|

// NOTE: Large portion of code copied from Project 1
// 		UDP server.c example.

// Required Libraries
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAX_MES_LEN 1024

void main (int argc, char *argv[]) {
    int sock, namelen, buflen, port;
    char srv_buf[MAX_MES_LEN];
    struct sockaddr_in name;

    // Ensure Proper Argument Usage
    if (argc != 2) {
		printf("ERROR: Incorrect number of arguements.");
		printf(" Please run using the following format...\n");
		printf("ftps <local-port>\n\n");
		exit(1);
	}
	port = atoi(argv[1]);

	// Ensure port is in range 1025-65535
	if (port < 1024 || port > 65535) {
		printf("ERROR: Invalid port value.");
		printf(" Please select a port between 1024-65535.\n");
		exit(1);
	}

    /*create socket*/
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
		perror("Error: Unable to open datagram socket");
		exit(1);
    }

    /* create name with parameters and bind name to socket */
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = INADDR_ANY;
    if(bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
		perror("Error: Unable to get socket name");
		exit(2);
    }
    namelen=sizeof(struct sockaddr_in);

    /* Find assigned port value and print it for client to use */
    if(getsockname(sock, (struct sockaddr *)&name, &namelen) < 0){
		perror("Error: Unable to get socket port");
		exit(3);
    }
    printf("Server Ready.\nPort: %d\n", ntohs(name.sin_port));

    /* waiting for connection from client on name and print what client sends */
    namelen = sizeof(name);
    buflen = MAX_MES_LEN;
    if(recvfrom(sock, srv_buf, buflen, 0, (struct sockaddr *)&name, &namelen) < 0) {
		perror("Error: Unable to recieve data"); 
		exit(4);
    }
    printf("Server receives: %s\n", srv_buf);

    /* server sends something back using the same socket */
    //bzero(srv_buf, MAX_MES_LEN);
    //strcpy(srv_buf, "Hello back in UDP from server");
    //printf("Server sends:    %s\n", srv_buf);
    //if(sendto(sock, srv_buf, buflen, 0, (struct sockaddr *)&name, namelen) < 0) {
	//	perror("sending datagram");
	//	exit(5);
    //}

    /* server terminates connection, closes socket, and exits */
    close(sock);
    exit(0);
}