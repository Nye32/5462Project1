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
#include "srbaclib.h"
#include <sys/time.h>

void * buffer;
uint32_t counter = 0;
fd_set read_fd;

void createConnection(int * hostSockfd, struct sockaddr_in * hostSockaddr, int  * remoteSockfd,struct sockaddr_in *remoteSockaddr, struct sockaddr_in * ftpsaddr)
{
	*hostSockfd = SOCKET(AF_INET, SOCK_DGRAM,IPPROTO_UDP);

	if(*hostSockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
		exit(0);
	}

	hostSockaddr->sin_family = AF_INET;
	hostSockaddr->sin_port = htons(atoi("5050"));
	hostSockaddr->sin_addr.s_addr = inet_addr("127.0.0.1");	
	memset(&(hostSockaddr->sin_zero),'\0',8);                                             	
	if(BIND(*hostSockfd, (struct sockaddr *) hostSockaddr, sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"%s\n","couldn't bind socket");
		exit(0);
	}

	*remoteSockfd = SOCKET(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
																							 
	if(*remoteSockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
		exit(0);
	}
																							 
	remoteSockaddr->sin_family = AF_INET;
	remoteSockaddr->sin_port = htons(atoi("6050"));
	remoteSockaddr->sin_addr.s_addr = inet_addr("127.0.0.1");	
	memset(&(remoteSockaddr->sin_zero),'\0',8); 
	if(BIND(*remoteSockfd, (struct sockaddr *) remoteSockaddr, sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"%s\n","couldn't bind socket");
		exit(0);
	}                                                                                       	
	ftpsaddr->sin_family = AF_INET;
    ftpsaddr->sin_port = htons(atoi("2000"));
    ftpsaddr->sin_addr.s_addr = inet_addr("127.0.0.1");	
	memset(&(ftpsaddr->sin_zero),'\0',8);   
	return;
}                                                                                     


void hostisset(int * hostSock, struct sockaddr_in * ftpsaddr)
{
	void * temp = malloc(4);
	fprintf(stderr, "%s\n", "got a call");
	if(RECV(*hostSock,temp,4,0) < 0)
	{
		fprintf(stderr, "%s\n", "failed to read max byte number sent from ftps...exiting...");
		exit(0);
	}


	uint32_t * int32 = temp;
	if(*int32>counter)
		*int32 = counter;
	dest_addr = *(struct sockaddr *)ftpsaddr;
		int sent = SEND(*hostSock, buffer, *int32, 0);
		if(sent < 0)
		{
			fprintf(stderr, "%s %s \n","couldn't send data to ftps ...exiting...",strerror(errno));
			exit(0);
		}
		memmove(buffer, buffer+sent, counter-sent);
		counter -= sent;
	free(temp);
	fprintf(stderr,"%d\n",counter);
	fprintf(stderr, "%s\n", "sent back");
}

void remoteisset(int * remoteSock)
{
	printf("%d\n",counter);
	if(counter<=64000)
	{
		int read = RECV(*remoteSock,buffer+counter,64000-counter,0);
		fprintf(stderr,"%d\n", read);
		if(read < 0)
		{
			fprintf(stderr,"%s\n","couldn't recv from remote...exiting...");
			exit(0);
		}
		counter += read;
	}
	fprintf(stderr,"%s\n", "got remote send");
}



void checkRead(int * hostSockfd, int * remoteSockfd, struct sockaddr_in * ftpsaddr)
{
	FD_ZERO(&read_fd);
	FD_SET(*hostSockfd, &read_fd);
	FD_SET(*remoteSockfd, &read_fd);
	struct timeval timeout;
	timeout.tv_sec = 10;	
	if(select(FD_SETSIZE, &read_fd,NULL,NULL,&timeout) < 0)
	{
		fprintf(stderr,"%s\n","select failed...exiting..." );
		exit(0);
	}
	if(FD_ISSET(*hostSockfd, &read_fd))
	{		
		hostisset(hostSockfd,ftpsaddr);
	}		
	if(FD_ISSET(*remoteSockfd, &read_fd))
	{
		remoteisset(remoteSockfd);
	}
}


int main(int args, char * argv[])
{
	int hostSockfd;
	int remoteSockfd;
	struct sockaddr_in hostSockaddr;
	struct sockaddr_in remoteSockaddr;
	struct sockaddr_in ftpsaddr;
	createConnection(&hostSockfd, &hostSockaddr, &remoteSockfd, &remoteSockaddr, &ftpsaddr);
	buffer = malloc(64000);
	for(;;)
	{
		checkRead(&hostSockfd, &remoteSockfd, &ftpsaddr);
	}
	fprintf(stdout, "%s %lu\n", "ended normally", sizeof(uint32_t));
	fprintf(stdout, "%s\n", "localhost port open on 127.0.0.1");
}




