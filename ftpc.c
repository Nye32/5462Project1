//lab2

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


//sending the file
void sendFile(int * sockfd, FILE * transFile, uint32_t fsize, char * filename)
{
	//creating the buffer and setting up the first SEND with the 24 byte header
	fprintf(stderr, "size of file is %d\n", fsize);
	void * buf = (void *) malloc(1000);
	bzero(buf,1000);
	uint32_t netfsize = htonl(fsize);
	memcpy(buf, &netfsize,sizeof(uint32_t));
	memcpy(buf+4,filename,strlen(filename));
	int read = 0;	
	read = fread(buf+24,1,1000-24,transFile);
	SEND(*sockfd,buf,read+24,0);

	
	//SEND bytes until the file descriptor reaches the end of the file
	while(feof(transFile) == 0)
	{
		read = fread(buf,1,1000,transFile);
		int sent = SEND(*sockfd,buf,read,0);
		usleep(3000 * 1000);
		if(sent != read)
		{
			fprintf(stdout, "sent %d Reason %s\n\n", sent, strerror(errno));
			fprintf(stderr,"%s\n","failed to send correctly");
			exit(0);
		}
	}

	//free the buffer
	free(buf);
	return;
}


//for the size and other details of the file being sent
unsigned long fileSize(const char *filePath)
{
 
 struct stat fileStat;//declare file statistics struct
 
 stat(filePath,  &fileStat);//filling fileStat with statistics of file
 
 return (uint32_t)fileStat.st_size;//returning only the size of file from stat stru    ct
}


//creating the connections used for SENDing the file through UDP
void createConnection(int * sockfd, struct sockaddr_in * sockaddr, char *port, char * serverIp)
{
	*sockfd = SOCKET(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
	if(*sockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
		exit(0);
	}
	
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons(atoi(port));
	sockaddr->sin_addr.s_addr = inet_addr(serverIp);	
	memset(&(sockaddr->sin_zero),'\0',8);


	setSendAddress(*((struct sockaddr*) sockaddr));
	return;
}

//main method
int main(int args, char * argv[])
{

	//aurgument checking
	if (args != 4)
	{
		fprintf(stderr,"%s %d %s\n" , "please give three aurguments,",args - 1,"were entered, the aurguments are remote-ip, remote-port, local-file-to-transfer");

	}
	
	//opening file
	FILE* transFile = fopen(argv[3],"r");//trying to open local-file-to-transfer  in read mode
	if(transFile == NULL)//Checking if input file was opened correctly
 	{
		fprintf(stdout,"%s","please enter a valid path to an existing file for the third argument\n");
		exit(0);//if not opened correctly, exit program
	}


	//getting size of the file
 	uint32_t fsize = fileSize(argv[3]);
	
	//created the socket for sending file and the sockaddr_in for the file	
	int sockfd;
	struct sockaddr_in sockaddr;		

	//creating the connections for sending the file
	createConnection(&sockfd, &sockaddr, argv[2], argv[1]);

	//finally sending the file
	sendFile(&sockfd,transFile,fsize,argv[3]);

	//printing out information if sent successfully
	fprintf(stdout, "%s\n", "ended normally");
}
