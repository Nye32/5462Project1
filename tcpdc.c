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




struct trollSock
{
	struct sockaddr_in remote;
	char buffer[1000];// = malloc(1000);
}pack;


void createConnection(struct sockaddr_in * trolladdr,int * hostSockfd, struct sockaddr_in * hostSockaddr, char * serverIp, char * serverPort)
{
	*hostSockfd = SOCKET(AF_INET, SOCK_DGRAM,IPPROTO_UDP);

	if(*hostSockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
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

	pack.remote.sin_family = AF_INET;
	pack.remote.sin_port = htons(atoi(serverPort));
	pack.remote.sin_addr.s_addr = inet_addr(serverIp);	
	memset(&(pack.remote.sin_zero),'\0',8);                                             	

	
	trolladdr->sin_family = AF_INET;
	trolladdr->sin_port = htons(atoi("4000"));
	trolladdr->sin_addr.s_addr = inet_addr("0.0.0.0");
	fprintf(stderr,"%u",inet_addr("127.0.0.1"));
	memset(&(trolladdr->sin_zero),'\0',8);                                             	

	return;
}                                                                                     


void receiveAndSend( struct sockaddr_in * trolladdr, int * hostSockfd)
{
	int read = RECV(*hostSockfd, pack.buffer, 1000, 0);
	fprintf(stderr,"%d\n",read);
	if(read < 0)
	{
		fprintf(stderr, "%s\n", "failed to read pack, discarding");	
	}
	else
	{
		perror("worked");
		dest_addr = *((struct sockaddr *)&pack.remote);
		fprintf(stderr, "%c\n",((char* )pack.buffer)[0] );
		if(SEND(*hostSockfd, pack.buffer, 1000/*sizeof(struct trollSock)*/, 0) < 0)
		{
			fprintf(stderr, "%s\n", "couldn't send...quiting...");
			exit(0);
		}
	}
}


int main(int args, char * argv[])
{
	if (args != 3)
	{
		fprintf(stderr,"%s %d %s\n" , "please give 2 aurguments,",args - 1,"were entered, the aurguments are remote-ip, remote-port");
		exit(0);

	}
	
	int hostSockfd;
	struct sockaddr_in hostSockaddr;
	struct sockaddr_in trolladdr;
	createConnection(&trolladdr, &hostSockfd, &hostSockaddr, argv[1], argv[2]);
	bzero(pack.buffer,1000);
	for(;;)
	{
		receiveAndSend(&trolladdr, &hostSockfd);
	}
	fprintf(stdout, "%s %lu\n", "ended normally", sizeof(uint32_t));
	fprintf(stdout, "%s\n", "localhost port open on 127.0.0.1");
}




