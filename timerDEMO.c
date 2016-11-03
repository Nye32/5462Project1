//___________________________
//| Andrew Lee				|
//| CSE5462	Project 1		|
//| 09-26-16 timerDEMO	|
//|_________________________|

// Timer Demo
// Required Libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "tcpapi.h"

#define BUFSIZE 1000

int timersockfd;
struct sockaddr_in cli_addr;
struct sockaddr_in expire;

// Establishes connection with host - Copied from Lab02
void createConnection(int * timersockfd, struct sockaddr_in * cli_addr)
{
	//*timersockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	*timersockfd = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(*timersockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
		exit(0);
	}
	
	cli_addr->sin_family = AF_INET;
	cli_addr->sin_port = htons(atoi("3132"));
	cli_addr->sin_addr.s_addr = inet_addr("127.0.0.1");	
	memset(&(cli_addr->sin_zero),'\0',8);

	// Expiration use
	expire.sin_family = AF_INET;
	expire.sin_port = htons(atoi("6520"));
	expire.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(&(expire.sin_zero), '\0',8);
	if(BIND(*timersockfd, (struct sockaddr *)&expire, sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"%s\n","couldn't bind socket");
		exit(0);
	}

	// Set SEND address
	setSendAddress(*(struct sockaddr *)cli_addr);

	return;
}

// Starts timer in timerprocess
void starttimer(double time, uint32_t byteSeqNum) {
	// convert double to two ints
	int itemp = time;
	double dtemp = time - itemp;
	uint32_t ltime = htonl(itemp);
	itemp = dtemp * 1000;
	uint32_t ldec = htonl(itemp);

	uint32_t flag = htonl(1);
	uint32_t lbyte = htonl(byteSeqNum);
	int isize = sizeof(uint32_t);
	int dsize = sizeof(double);
	char * buf = (char *) malloc(4*isize);
	bzero(buf,(4*isize));
	memcpy(buf, &flag,isize);
	memcpy(buf+isize,&lbyte,isize);
	memcpy(buf+(2*isize), &ltime,isize);
	memcpy(buf+(3*isize), &ldec,isize);
	//sendto(timersockfd,buf,(4*isize),0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	SEND(timersockfd,buf,(4*isize),0);
	printf("SENT\nFlag: %d\nByte: %d\nTime: %d.%d\n", ntohl(flag), ntohl(lbyte), ntohl(ltime), ntohl(ldec));
}

// Cancels timer in timerprocess
void canceltimer(uint32_t byteSeqNum) {
	uint32_t flag = htonl(2);
	uint32_t lbyte = htonl(byteSeqNum);
	int isize = sizeof(uint32_t);
	char * buf = (char *) malloc(2*isize);
	bzero(buf,2*isize);
	memcpy(buf, &flag,isize);
	memcpy(buf+isize, &lbyte,isize);
	//sendto(timersockfd,buf,(4*isize),0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	SEND(timersockfd, buf, (4*isize), 0);
	printf("SENT\nFlag: %d\nByte: %d\n", ntohl(flag), ntohl(lbyte));
}

// Sends quit flag to timer
void timerquit() {
	uint32_t flag = htonl(3);
	int isize = sizeof(uint32_t);
	int dsize = sizeof(double);
	char * buf = (char *) malloc(2*isize);
	bzero(buf,2*isize);
	memcpy(buf, &flag,isize);
	//sendto(timersockfd,buf,(4*isize),0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	SEND(timersockfd, buf, (4*isize), 0);
	printf("SENT\nFlag: %d\n", ntohl(flag));
}

void main (int args, char *argv[]) {

	// Ensure proper argument usage
	// if (args != 3)
	// {
	// 	printf("Run using the following:\n\n\ttd <remote-ip> <remote-port>\n");
	// 	exit(0);
	// }

	// Establish Connection with tp
	createConnection(&timersockfd, &cli_addr);

	starttimer(20.32,1);
	starttimer(10.050,2);
	starttimer(30.0,3);

	while(1) {
		//read file as it is received
		char eByte[sizeof(int)];
		//int read = recvfrom(timersockfd, eByte, sizeof(int),0, NULL, NULL);
		int read = RECV(timersockfd, eByte, sizeof(int), 0);
		if(read < 0)
		{
			fprintf(stderr, "%s\n", "failed to read pack, discarding");	
		}
		printf("%d Expired.\n", ntohl(*((int *)(eByte))));
	}

	// starttimer(20.32,1);
	// starttimer(10.050,2);
	// starttimer(30.0,3);
	// canceltimer(1);
	// usleep(32000);
	// canceltimer(3);
	// starttimer(20.0,4);
	// sleep(6);
	// starttimer(18.0,5);
	// canceltimer(5);
	// canceltimer(8);
	// timerquit();
}
