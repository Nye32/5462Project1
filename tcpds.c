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
#include <sys/time.h>
#include "crc32.h"
#include "sbuffer.h"


//setting up global variables
void * buffer;
char * minibuffer;
int minibufferSize = 0;
uint32_t counter = 0;
fd_set read_fd;


struct  sockaddr_in trolladdr;

struct trollSock
{
	struct sockaddr_in remote;
	uint32_t ack;
}pack;



//creating the conenctions required
void createConnection(int * hostSockfd, struct sockaddr_in * hostSockaddr, int  * remoteSockfd,struct sockaddr_in *remoteSockaddr, struct sockaddr_in * ftpsaddr, struct sockaddr_in * trolladdr, char * serverAddress)
{
	*hostSockfd = SOCKET(AF_INET, SOCK_DGRAM,IPPROTO_UDP);

	if(*hostSockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
		exit(0);
	}

	hostSockaddr->sin_family = AF_INET;
	hostSockaddr->sin_port = htons(atoi("5050"));
	hostSockaddr->sin_addr.s_addr = INADDR_ANY;	
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
	remoteSockaddr->sin_addr.s_addr = INADDR_ANY;	
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

	pack.remote.sin_family = htons(AF_INET);
	pack.remote.sin_port = htons(atoi("6000"));
	pack.remote.sin_addr.s_addr = inet_addr(serverAddress);
	memset(&(pack.remote.sin_zero), '\0',8);

	trolladdr->sin_family = AF_INET;
	trolladdr->sin_port = htons(atoi("4000"));
	trolladdr->sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(&(trolladdr->sin_zero), '\0',8);

	return;

}

//method for if ftps is requesting bytes
void hostisset(int * hostSock, struct sockaddr_in * ftpsaddr)
{
	//creating buffer to hold request number
	void * temp = malloc(4);
	if(RECV(*hostSock,temp,4,0) < 0)
	{
		fprintf(stderr, "%s\n", "failed to read max byte number sent from ftps...exiting...");
		exit(0);
	}

	//checking if request asked for more than we have
	uint32_t * request_size = temp;
	char data[*request_size];

	int read = requestData(*request_size, data);

	//setting up to send packet to ftps
	setSendAddress(*(struct sockaddr *)ftpsaddr);
	
	int sent = SEND(*hostSock, data, read, 0);
	if(sent < 0)
	{
		fprintf(stderr, "%s %s \n","couldn't send data to ftps ...exiting...",strerror(errno));
		exit(0);
	}

	//freeing buffer
	free(temp);
}


//remote host(troll) is sending data to store
void remoteisset(int * remoteSock, struct sockaddr_in * trolladdr)
{
	char * tempbuf;
	char buffer[20*1024];
	//reading bytes
	int read = RECV(*remoteSock,buffer,sizeof(buffer),0);

	//checking the number of bytes read
	if(read < 0)
	{
		fprintf(stderr,"%s\n","couldn't recv from remote...exiting...");
		exit(0);
	}

	// check to see if end packet (non 1024 size)
	int lastPacket = read % 1024;

	// for loop for each packet
	for(int i = read/1024; i > 0; i--) {
		int offset = ((read/1024) - i) * 1024;
		tempbuf = buffer + offset;
		//checking the check sum
		uint32_t * crccheckval = (uint32_t * )(tempbuf + 16);
		uint32_t crcpoly = 0x04C11DB7;
		uint32_t retcrc = crc32(crcpoly, tempbuf + 24, read-24);		
		fprintf(stderr,"retcrc = %d\n",*crccheckval);
		fprintf(stderr, "checksum = %d\n",retcrc);


		//dealing with bad packet
		if(retcrc == *crccheckval)	
			fprintf(stderr,"\nCHECK SUM GOOD\n");
		else
			fprintf(stderr,"\nBAD CHECK SUM\n");

		uint32_t *packetnumber =(uint32_t *) (tempbuf + 20);
		fprintf(stdout,"\npacketnumber = %d\n",*packetnumber );

		// attempt to write to sbuffer
		int result = 0;
		if (retcrc == *crccheckval) {
			result = addData(*packetnumber, 1000, tempbuf + 24);
		}

		// if ACK is to be sent
		if (result != 0) {
			setSendAddress(*(struct sockaddr *) trolladdr);	
			
			memcpy(&(pack.ack), packetnumber, sizeof(uint32_t));

			if(SEND(*remoteSock,(char *)&pack, sizeof(struct trollSock), 0) < 0)
			{
				fprintf(stderr,"\n can't send ack, Quitting....., errno is %s\n",strerror(errno));
				exit(0);
			}
		}

		fprintf(stderr,"read %d\n\n", read);
	}
	printList();

	// Last packet
	if  (lastPacket != 0) {
		//checking the check sum
		uint32_t * crccheckval = (uint32_t * )(tempbuf + 16);
		uint32_t crcpoly = 0x04C11DB7;
		uint32_t retcrc = crc32(crcpoly, tempbuf-lastPacket, lastPacket);		
		fprintf(stderr,"retcrc = %d\n",*crccheckval);
		fprintf(stderr, "checksum = %d\n",retcrc);

				//dealing with bad packet
		if(retcrc == *crccheckval)	
			fprintf(stderr,"\nCHECK SUM GOOD\n");
		else
			fprintf(stderr,"\nBAD CHECK SUM\n");

		uint32_t *packetnumber =(uint32_t *) (tempbuf - lastPacket + 20);
		fprintf(stdout,"\npacketnumber = %d\n",*packetnumber );

		// attempt to write to sbuffer
		int result = 0;
		if (retcrc == *crccheckval) {
			result = addData(*packetnumber, lastPacket, tempbuf - lastPacket + 24);
		}

		// if ACK is to be sent
		if (result != 0) {
			setSendAddress(*(struct sockaddr *) trolladdr);	
			
			memcpy(&(pack.ack), packetnumber, sizeof(uint32_t));

			if(SEND(*remoteSock,(char *)&pack, sizeof(struct trollSock), 0) < 0)
			{
				fprintf(stderr,"\n can't send ack, Quitting....., errno is %s\n",strerror(errno));
				exit(0);
			}
		}

	}
}


//scanning both port for incoming data using select function
void checkRead(int * hostSockfd, int * remoteSockfd, struct sockaddr_in * ftpsaddr, struct sockaddr_in * trolladdr)
{
	FD_ZERO(&read_fd);
	FD_SET(*hostSockfd, &read_fd);
	FD_SET(*remoteSockfd, &read_fd);
	struct timeval timeout;
	timeout.tv_usec = 10*1000;	
	if(select(FD_SETSIZE, &read_fd,NULL,NULL,NULL) < 0)
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
		remoteisset(remoteSockfd, trolladdr);
	}
}


//main method
int main(int args, char * argv[])
{


	if(args < 2)
	{
		fprintf(stderr,"need server address of tcpdc");
		exit(0);
	}
	
	// Initialize sbuffer
	initialize();


	//socket and struct sockaddr declaration
	int hostSockfd;
	int remoteSockfd;
	struct sockaddr_in hostSockaddr;
	struct sockaddr_in remoteSockaddr;
	struct sockaddr_in ftpsaddr;
	struct sockaddr_in trolladdr;
	
	//creating the connections for sending and receiving
	createConnection(&hostSockfd, &hostSockaddr, &remoteSockfd, &remoteSockaddr, &ftpsaddr, &trolladdr, argv[1]);

	//allocating space for buffer and minibuffer
	buffer = malloc(64000);
	minibuffer = malloc(20 * 1024);
	bzero(buffer, sizeof(buffer));
	bzero(minibuffer, sizeof(minibuffer));

	//while loop for deamon like feel
	for(;;)
	{
		checkRead(&hostSockfd, &remoteSockfd, &ftpsaddr, &trolladdr);
	}

	return 1;
}


