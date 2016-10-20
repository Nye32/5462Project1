//TODO


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
#include "crc32.h"
#include "cbuffer.h"
#include <math.h>
#include <time.h>

double SRTT = 0;
double RTTVAR = 0;
double RTO = 1000;


//creating the troll header
struct trollSock
{
	struct sockaddr_in remote;
	uint32_t crc32;
	uint32_t packnum;
	char buffer[1000];// = malloc(1000);
}pack;


//for the necessary connections required for sending the file and receiving the file
void createConnection(struct sockaddr_in * trolladdr,int * hostSockfd,int * remoteSockfd, struct sockaddr_in * hostSockaddr, struct sockaddr_in * ackret, char * serverIp)
{
	*hostSockfd = SOCKET(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
	*remoteSockfd = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(*hostSockfd < 0 || *remoteSockfd < 0)
	{
		fprintf(stdout, "%s\n", "sockets could not be made");
		exit(0);
	}

	hostSockaddr->sin_family = AF_INET;
	hostSockaddr->sin_port = htons(atoi("5000"));
	hostSockaddr->sin_addr.s_addr = inet_addr("127.0.0.1");	
	memset(&(hostSockaddr->sin_zero),'\0',8);      	
	if(BIND(*hostSockfd, (struct sockaddr *) hostSockaddr, sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"%s\n","couldn't bind socket");
		exit(0);
	}


	ackret->sin_family = AF_INET;
	ackret->sin_port = htons(atoi("6000"));
	ackret->sin_addr.s_addr = inet_addr("127.0.0.1");	
	memset(&(ackret->sin_zero),'\0',8);      	
	if(BIND(*remoteSockfd, (struct sockaddr *) ackret, sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"%s\n","couldn't bind socket");
		exit(0);
	}

	pack.remote.sin_family = htons(AF_INET);
	pack.remote.sin_port = htons(atoi("6050"));
	pack.remote.sin_addr.s_addr = inet_addr(serverIp);	
	memset(&(pack.remote.sin_zero),'\0',8);                                             	


	
	trolladdr->sin_family = AF_INET;
	trolladdr->sin_port = htons(atoi("4000"));
	trolladdr->sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(&(trolladdr->sin_zero),'\0',8);                                             	

	return;
}                                                                                     


void timeComps(double delta)
{
	if(SRTT == 0)
	{
		SRTT = delta;
	}
	if(RTTVAR == 0)
	{
		RTTVAR = (delta/2);
	}
	else
	{
		SRTT = ((1 - (1/8) ) * SRTT) + ((1/8) * delta);
		RTTVAR = ((1 - (1/4)) * RTTVAR) + ((1/4) * fabs(SRTT - delta));
		RTO = SRTT + (4 * RTTVAR);
	}

	fprintf(stderr, "SRTT = %f, RTTVAR = %f, RTO = %f\n", SRTT, RTTVAR, RTO);

}




//receiving and sending the file
void receiveAndSend( struct sockaddr_in * trolladdr, int * hostSockfd, int * remoteSockfd)
{
	//read file as it is received
	char tem[1000];
	int read = RECV(*hostSockfd, tem, 1000, 0);
	fprintf(stderr,"\nreceived %d\n",read);
	if(read < 0)
	{
		fprintf(stderr, "%s\n", "failed to read pack, discarding");	
	}
	else
	{

		addData(tem, read);
		
		
		int num = requestBSN();
		int size = requestSize(num);
		char data[size];
		requestData(num, data);

		memcpy(pack.buffer,data, size);


		//do the crc checksumming
		uint32_t crcpoly = 0x04C11DB7;
		pack.crc32 = crc32(crcpoly,pack.buffer,size);

		//printing checksum
		fprintf(stderr,"checksum = %d\n",pack.crc32);

		
		memcpy(&(pack.packnum), &num, sizeof(uint32_t));
		fprintf(stderr,"packetnumber = %d\n", num);


		//Sending the changed packet forward to troll
		setSendAddress(*(struct sockaddr *) trolladdr);

		struct timespec start, stop;
		clock_gettime(CLOCK_MONOTONIC, &start);

		if(SEND(*remoteSockfd, (char *)&pack, size+sizeof(struct trollSock)-1000, 0) < 0)
		{
			fprintf(stderr, "%s %s \n", "couldn't send...quiting...", strerror(errno));
			exit(0);
		}
		char buffer[20];
		RECV(*remoteSockfd,buffer, sizeof(buffer),0 );

		clock_gettime(CLOCK_MONOTONIC, &stop);

		double  delta = ((stop.tv_sec - start.tv_sec) * 1000.0) + ((stop.tv_nsec - start.tv_nsec) / 1000000.0);
		fprintf(stderr,"delta = %f\n",delta);
		timeComps(delta);

		uint32_t * ack = (uint32_t *)(buffer + 16);
		fprintf(stderr,"returned ack is %u\n",*ack);
		recvACK(*ack);
	}
}


//main method
int main(int args, char * argv[])
{

	initHead();
	//checking the aurguments
	if (args != 2)
	{
		fprintf(stderr,"%s %d %s\n" , "please give 1 aurgument,",args - 1,"were entered, the aurgument is remote-ip");
		exit(0);

	}
	
	//creating the socket descritor and the sockaddrs for forwarding the packet
	int hostSockfd;
	int remoteSockfd;
	struct sockaddr_in hostSockaddr;
	struct sockaddr_in trolladdr;
	struct sockaddr_in ackret;
	//creating the connectio ns required for UDP SEND
	createConnection(&trolladdr, &hostSockfd, &remoteSockfd, &hostSockaddr, &ackret, argv[1]);

	//zeroing the buffer just for safety
	bzero(pack.buffer,1000);

	//while loop for daemon type feel for constantly receiving and sending data packets through while modifying them
	for(;;)
	{
		receiveAndSend(&trolladdr, &hostSockfd, &remoteSockfd);
	}

	return 1;
}





