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


//creating the troll header
struct trollSock
{
	struct sockaddr_in remote;
	uint32_t crc32;
	char buffer[1000];// = malloc(1000);
}pack;


//for the necessary connections required for sending the file and receiving the file
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



	pack.remote.sin_family = htons(AF_INET);
	pack.remote.sin_port = htons(atoi(serverPort));
	pack.remote.sin_addr.s_addr = inet_addr(serverIp);	
	memset(&(pack.remote.sin_zero),'\0',8);                                             	


	
	trolladdr->sin_family = AF_INET;
	trolladdr->sin_port = htons(atoi("4000"));
	trolladdr->sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(&(trolladdr->sin_zero),'\0',8);                                             	

	return;
}                                                                                     

//receiving and sending the file
void receiveAndSend( struct sockaddr_in * trolladdr, int * hostSockfd)
{
	//read file as it is received
	int read = RECV(*hostSockfd, pack.buffer, 1000, 0);
	fprintf(stderr,"received %d\n",read);
	if(read < 0)
	{
		fprintf(stderr, "%s\n", "failed to read pack, discarding");	
	}
	else
	{
		//do the crc checksumming
		uint32_t crcpoly = 0x04C11DB7;
		pack.crc32 = crc32(crcpoly,pack.buffer,read);

		//printing checksum
		fprintf(stderr,"checksum = %d\n",pack.crc32);

		//Sending the changed packet forward to troll
		setSendAddress(*(struct sockaddr *) trolladdr);
		if(SEND(*hostSockfd, (char *)&pack, read+sizeof(struct sockaddr_in) + sizeof(uint32_t), 0) < 0)
		{
			fprintf(stderr, "%s %s \n", "couldn't send...quiting...", strerror(errno));
			exit(0);
		}
	}
}


//main method
int main(int args, char * argv[])
{
	//checking the aurguments
	if (args != 3)
	{
		fprintf(stderr,"%s %d %s\n" , "please give 2 aurguments,",args - 1,"were entered, the aurguments are remote-ip, remote-port");
		exit(0);

	}
	
	//creating the socket descritor and the sockaddrs for forwarding the packet
	int hostSockfd;
	struct sockaddr_in hostSockaddr;
	struct sockaddr_in trolladdr;

	//creating the connectio ns required for UDP SEND
	createConnection(&trolladdr, &hostSockfd, &hostSockaddr, argv[1], argv[2]);

	//zeroing the buffer just for safety
	bzero(pack.buffer,1000);

	//while loop for daemon type feel for constantly receiving and sending data packets through while modifying them
	for(;;)
	{
		receiveAndSend(&trolladdr, &hostSockfd);
	}

	return 1;
}





