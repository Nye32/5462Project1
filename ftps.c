//_______________________
//| Andrew Lee			|
//| CSE5462	Lab 03		|
//| 09-05-16	ftps.c	|
//|_____________________|

// NOTE: Large portion of code copied from Project 1
// 		TCP server.c example.

// Required Libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "tcpapi.h"

#define BUFSIZE 1000


//main method
int main (int argc, char *argv[]) {
	
	// Variables
	int sock;
	int rval = 0;
	int filesize = 0;
	char filename[20] = "";
	struct sockaddr_in sin_addr;
	char databufin[BUFSIZE];
	struct sockaddr_in request;
	uint32_t request_size = 4;
	
	
	
	// Wait for client connection
	if ((sock = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("Error: Unable to Open Datagram Socket.");
		exit(1);
	}
	
	// Construct sent socket
	sin_addr.sin_family = AF_INET;
	sin_addr.sin_addr.s_addr = INADDR_ANY; 
	sin_addr.sin_port = htons(atoi("2000"));
	memset(&(sin_addr.sin_zero), '\0',8);

	// Bind Socket
	BIND(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in));



	//for requesting bytes
	request.sin_family = AF_INET;
	request.sin_port = htons(atoi("5050"));
	request.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(&(request.sin_zero), '\0',8);
	setSendAddress( *((struct sockaddr *)&request));

	
	printf("UDP Server Initialized. Awaiting Clients...\n");



	// Clear Buffer (Set it to 0)
	bzero(databufin, sizeof(databufin));

	
	// Read Header
	rval = 24;
	request_size = 24;
	while (request_size > 0) {

		//sending request for bytes
		if(SEND(sock, &request_size, sizeof(uint32_t), 0) < 0)
		{
			fprintf(stderr,"%s\n", "failed to send request...exiting...");
			exit(0);
		}
		request_size -= RECV(sock, databufin+(rval-request_size), request_size, 0);
	}


	//confirming header as sent
	fprintf(stderr,"got header\n");
	

	// Determine Size of file from header
	char temp[20];
	unsigned int i = 0;
	for (i = 0; i < sizeof(uint32_t); i++) {
		temp[i] = databufin[i];
	}
	filesize = ntohl(*((uint32_t *)(temp)));
	printf("Filesize: %d\n", filesize);
	

	// Determine Filename
	for (i = 0; i < 20; i++) {
		temp[i] = databufin[i+sizeof(uint32_t)];
	}
	printf("Filename: %s\n", temp);
	filename[0] = 'r';
	filename[1] = 'e';
	filename[2] = 'c';
	filename[3] = 'v';
	filename[4] = 'd';
	filename[5] = '/';
	strcat(filename,temp);
	printf("Filename: %s\n", filename);
	

	//make recvd folder if it doesn't exist
	struct stat st;
	if(stat("recvd",&st) != 0)
	{
		mkdir("recvd",0700);
	}


	// Output File
	FILE *oufp;
	oufp = fopen(filename, "w+");
	if (oufp == NULL) {
		perror("Error: Unable to create output file.");
	}


	// Recieve and Write Data
	rval = 0;
	int tempval = 0;
	request_size = 25;
	while (rval < filesize) {	
	
		//requesting for bytes
		if(SEND(sock, &request_size, sizeof(uint32_t), 0) < 0)
		{
			fprintf(stderr,"%s\n", "failed to send request...exiting...");
			exit(0);
		}
		
		fprintf(stderr,"rval  = %d\n",rval);
		fprintf(stderr,"requestsize = %d\n",request_size);
		
		//waiting for receive from the request
		tempval = RECV(sock, databufin, request_size, 0);
		
		//increamenting counters for receiving file correctly
		rval += tempval;
		if (tempval  < 0) {
			perror("Error: Unable to read Stream Socket.");
			exit(1);
		}
		
		// Write to Output File
		fwrite(databufin, 1, tempval, oufp);
	}


	//printing number of bytes gotten for whole file	
	fprintf(stderr,"complete non-header receive size = %d\n", rval);
	
	
	// Close File/Connections
	fclose(oufp);
	close(sock);
}
