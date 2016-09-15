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


void createConnection(int * remoteSockfd, struct sockaddr_in * remoteSockaddr, int * hostSockfd, struct sockaddr_in * hostSockaddr, char *remotePort, char * serverIp, char * hostPort)
{
	*remoteSockfd = SOCKET(AF_INET, SOCK_DGRAM,0);
	if(*remoteSockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
		exit(0);
	}
	
	remoteSockaddr->sin_family = AF_INET;
	remoteSockaddr->sin_port = htons(atoi(remotePort));
	remoteSockaddr->sin_addr.s_addr = inet_addr(serverIp);	
	memset(&(remoteSockaddr->sin_zero),'\0',8);
	if(BIND(*remoteSockfd,(struct sockaddr *)remoteSockaddr,sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "%s failed becuase %s\n", "failed to connect to remote host", strerror(errno));
		close(*remoteSockfd);
		exit(0);
	}
	*hostSockfd = SOCKET(AF_INET, SOCK_DGRAM,0);

	if(*hostSockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
		exit(0);
	}

	hostSockaddr->sin_family = AF_INET;
	hostSockaddr->sin_port = htons(atoi(hostPort));
	hostSockaddr->sin_addr.s_addr = inet_addr("127.0.0.1");	
	memset(&(hostSockaddr->sin_zero),'\0',8);                                             	
	if(BIND(*hostSockfd,(struct sockaddr *)hostSockaddr,sizeof(struct sockaddr_in)) < 0)
	{                                                                                       
		fprintf(stderr, "%s failed becuase %s\n", "failed to bind to host socket", strerror(errno));
		close(*hostSockfd);
		exit(0);
	}  

	return;
}                                                                                     


void recieveAndSend( int * remoteSockfd, int * hostSockfd, char * buffer, int bufferLen)
{

	if(REVC(*hostSockfd, buffer, bufferLen, 0) < 0)
	{
		fprintf(stderr, "%s\n", "failed to read pack, discarding");	
	}
	else
	{
		SEND(*remoteSockfd, buffer, bufferLen, 0);
	}
}


int main(int args, char * argv[])
{
	if (args != 4)
	{
		fprintf(stderr,"%s %d %s\n" , "please give three aurguments,",args - 1,"were entered, the aurguments are remote-ip, remote-port, host-port");

	}

	int hostSockfd;
	int remoteSockfd;
	struct sockaddr_in remoteSockaddr;		
	struct sockaddr_in hostSockaddr;
	createConnection(&remoteSockfd, &remoteSockaddr, &hostSockfd, &hostSockaddr, argv[2], argv[1], argv[3]);
	fprintf(stdout, "%s %lu\n", "ended normally", sizeof(uint32_t));
	fprintf(stdout, "%s\n", "localhost port open on 127.0.0.1");
}
