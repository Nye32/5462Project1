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

// srbac Library
#include "srbac.h"

#define BUFSIZE 1000

void main (int argc, char *argv[]) {
	// Variables
	int port;
	int sock;
	int msgsock;
	int rval = 0;
	int filesize = 0;
	char filename[20] = "";
	struct sockaddr_in sin_addr;
	char databufin[BUFSIZE];
	char databufout[BUFSIZE] = "Server Response: hi";
	
	// Ensure Proper Argument Usage
	if (argc != 2) {
		printf("ERROR: Incorrect number of arguements.");
		printf(" Please run using the following format...\n");
		printf("ftps <local-port>\n\n");
		exit(1);
	}
	port = atoi(argv[1]);
	
	// Wait for client connection
	printf("TCP Server Initialized. Awaiting Clients...\n");
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error: Unable to Open Datagram Socket.");
		exit(1);
	}
	
	// Construct sent socket
	sin_addr.sin_family = AF_INET;
	sin_addr.sin_addr.s_addr = INADDR_ANY;
	sin_addr.sin_port = htons(port);
		

	// Bind Socket
	BIND(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in));
	/*
	if (bind(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("Error: Unable to Bind Stream Socket");
		exit(1);
	}
	*/
	
	// Listen for socket connections (Max open connections is 5)
	listen(sock, 5);
	
	// Accept 1 connection: msgsocket
	if ((msgsock = accept(sock, (struct sockaddr *)NULL, (int *)NULL)) == -1) {
		perror("Error: Unable to connect Stream Socket.");
		exit(1);
	}
	
	// Clear Buffer (Set it to 0)
	bzero(databufin, sizeof(databufin));
	
	// Read Header
	rval = 24;
	while (rval > 0) {
		rval -= recv(msgsock, databufin, rval, 0);
	}
	
	// Determine Size of file from header
	char temp[20];
	int i = 0;
	for (i = 0; i < 4; i++) {
		temp[i] = databufin[i];
	}
	filesize = ntohl(*((uint32_t *)(temp)));
	printf("Filesize: %d\n", filesize);
	
	// Determine Filename
	for (i = 0; i < 20; i++) {
		temp[i] = databufin[i+4];
	}
	strcpy(filename, temp);
	printf("Filename: %s\n", filename);
	filename[0] = 'r';
	filename[1] = 'e';
	filename[2] = 'c';
	filename[3] = 'v';
	filename[4] = 'd';
	filename[5] = '/';
	strcat(filename,temp);
	printf("Filename: %s\n", filename);
	
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
	while (rval < filesize) {
		tempval = recv(msgsock, databufin, BUFSIZE, 0);
		rval += tempval;
		if (tempval  < 0) {
			perror("Error: Unable to read Stream Socket.");
			exit(1);
		}

		// Write to Output File
		fwrite(databufin, 1, tempval, oufp);
		//fprintf(oufp, "%s", databufin); //Troubleshooting
	}
	
	// Close File/Connections
	fclose(oufp);
	close(msgsock);
	close(sock);
}