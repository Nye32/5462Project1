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

int hostSockfd;
int remoteSockfd;
struct sockaddr_in hostSockaddr;
struct sockaddr_in ackret;
struct sockaddr_in sendhost;

fd_set read_fd;
int tempStored = 0;
int bread = 0;



//for the necessary connections required for sending the file and receiving the file
void createConnection(int * hostSockfd,int * remoteSockfd, struct sockaddr_in * hostSockaddr, struct sockaddr_in * ackret, struct sockaddr_in * sendhost)
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

	
	sendhost->sin_family = AF_INET;
	sendhost->sin_port = htons(atoi("3000"));
	sendhost->sin_addr.s_addr = inet_addr("127.0.0.1");	
	memset(&(sendhost->sin_zero),'\0',8);      	
	

	return;
}                                                                                     
int x = 0;

//receiving and sending the file
void receiveAndSend()
{
	FD_ZERO(&read_fd);
	FD_SET(hostSockfd, &read_fd);
	FD_SET(remoteSockfd, &read_fd);
	struct timeval timeout;
	timeout.tv_usec = 1 * 1000;	
	timeout.tv_sec = 0;
	char temp[1000];
	if(select(FD_SETSIZE, &read_fd,NULL,NULL,&timeout) < 0)
	{
		fprintf(stderr,"%s\n","select failed...exiting..." );
		exit(0);
	}
	if(tempStored)
	{
		if(addData(temp, bread) == 1)
			{
				setSendAddress(*(struct sockaddr *) &sendhost);
				char ret[] = {'1'};
				SEND(hostSockfd,ret, 1, 0);	
				tempStored = 0;
			}
		else
		{
			perror("still failing\n");
			tempStored = 1;
		}	

	}
	if(FD_ISSET(hostSockfd, &read_fd) && tempStored == 0)
	{
		bread = RECV(hostSockfd, temp, 1000, 0);
		fprintf(stderr,"\nreceived %d\n",bread);
		if(bread < 0)
		{
			fprintf(stderr, "%s\n", "failed to read pack, discarding");	
		}				
		else
		{

			if(addData(temp, bread) == 1)
			{
				setSendAddress(*(struct sockaddr *) &sendhost);
				char ret[] = {'1'};
				SEND(hostSockfd,ret, 1, 0);	
				tempStored = 0;
			}
			else
			{
				perror("Too Full");
				tempStored = 1;			
			}
		
		}
	}
	if(FD_ISSET(remoteSockfd, &read_fd))
	{
		char buffer[1000];
		int recvd = RECV(remoteSockfd,buffer, sizeof(buffer),0 );
		int nums = recvd/(sizeof(struct sockaddr_in) + sizeof(uint32_t));
		for(int i = 0; i<nums; i++)
		{
			uint32_t * ack = (uint32_t *)((buffer + 16) + ( i * (sizeof(struct sockaddr_in) + sizeof(uint32_t))));
			fprintf(stderr,"Returned ACK is %u\n",*ack);
			recvACK(*ack);
		}

	}

	if(x % 64 ==  0)
		sendWindow();
	x++;
}


//main method
int main(int args, char * argv[])
{

	//checking the aurguments
	if (args != 2)
	{
		fprintf(stderr,"%s %d %s\n" , "please give 1 aurgument,",args - 1,"were entered, the aurgument is remote-ip");
		exit(0);

	}
	
	createConnection(&hostSockfd, &remoteSockfd, &hostSockaddr, &ackret, &sendhost);
	initialize(argv[1], remoteSockfd);
	
	//while loop for daemon type feel for constantly receiving and sending data packets through while modifying them
	for(;;)
	{
		receiveAndSend();
	}

	return 1;
}


