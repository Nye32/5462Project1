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


//setting up global variables
void * buffer;
uint32_t counter = 0;
fd_set read_fd;


//creating the conenctions required
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
	if(*request_size>counter)
		*request_size = counter;

	//setting up to send packet to ftps
	setSendAddress(*(struct sockaddr *)ftpsaddr);

	//sending the data
	int sent = SEND(*hostSock, buffer, *request_size, 0);
	if(sent < 0)
	{
		fprintf(stderr, "%s %s \n","couldn't send data to ftps ...exiting...",strerror(errno));
		exit(0);
	}

	//moving data in buffer so that sent data wont be resent and reseting counter of number of bytes available to send
	memmove(buffer, buffer+sent, counter-sent);
	counter -= sent;

	//freeing buffer
	free(temp);
}


//remote host(troll) is sending data to store
void remoteisset(int * remoteSock)
{

	//printing how much counter is
	printf("counter = %d\n",counter);

	//making a temporary buffer to hold sent data in to modify it
	char tempbuf[1020];
	int rval = 1020;

	//checking of the buffer for data storage is all adapting number of bytes to read for it
	if(counter<=64000)
	{

		if((64000-counter)<1020)
			rval = 64000-counter+20;
		else
			rval = 1020;
		
		
		//reading bytes
		int read = RECV(*remoteSock,tempbuf,rval,0);
		
		//checking the check sum
		uint32_t * crccheckval = (uint * )(tempbuf + 16);
		uint32_t crcpoly = 0x04C11DB7;
		uint32_t retcrc = crc32(crcpoly, tempbuf + 20, read-20);		
		fprintf(stderr,"retcrc = %d\n",*crccheckval);
		fprintf(stderr, "checksum = %d\n",retcrc);
		
		//dealing with bad packet
		if(retcrc == *crccheckval)	
			fprintf(stderr,"\n\nCHECK SUM GOOD\n\n");
		else
			fprintf(stderr,"\n\nBAD CHECK SUM\n\n");

		//copying file sent file data into buffer	
		memcpy(buffer+counter,tempbuf+20,read-20);
		fprintf(stderr,"read %d\n", read);
		
		//checking the number of bytes read
		if(read < 0)
		{
			fprintf(stderr,"%s\n","couldn't recv from remote...exiting...");
			exit(0);
		}

		//incrementing counter by number of data bytes written to buffer
		counter += read-20;
	}
}


//scanning both port for incoming data using select function
void checkRead(int * hostSockfd, int * remoteSockfd, struct sockaddr_in * ftpsaddr)
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
		remoteisset(remoteSockfd);
	}
}


//main method
int main(int args, char * argv[])
{
	//socket and struct sockaddr declaration
	int hostSockfd;
	int remoteSockfd;
	struct sockaddr_in hostSockaddr;
	struct sockaddr_in remoteSockaddr;
	struct sockaddr_in ftpsaddr;

	//creating the connections for sending and receiving
	createConnection(&hostSockfd, &hostSockaddr, &remoteSockfd, &remoteSockaddr, &ftpsaddr);

	//allocating space for buffer
	buffer = malloc(64000);

	//while loop for deamon like feel
	for(;;)
	{
		checkRead(&hostSockfd, &remoteSockfd, &ftpsaddr);
	}

	return 1;
}

