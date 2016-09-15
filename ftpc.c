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
#include "srbaclib.h"

void sendFile(int * sockfd, FILE * transFile, uint32_t fsize, char * filename)
{
	fprintf(stderr, "size of file is %d\n", fsize);
	char * buf = (char *) malloc(1000);
	bzero(buf,1000);
	uint32_t netfsize = htonl(fsize);
	memcpy(buf, &netfsize,sizeof(uint32_t));
	memcpy(buf+4,filename,strlen(filename));
	SEND(*sockfd,buf,24,0);
	int read = 0;	
	while(feof(transFile) == 0)
	{
		read = fread(buf,1,1000,transFile);
		int sent = SEND(*sockfd,buf,read,0);
		if(sent != read)
		{
			fprintf(stdout, "sent %d Reason %s\n\n", sent, strerror(errno));
			fprintf(stderr,"%s\n","failed to send correctly");
			exit(0);
		}
	}
	free(buf);
	return;
}

unsigned long fileSize(const char *filePath)
{
 
 struct stat fileStat;//declare file statistics struct
 
 stat(filePath,  &fileStat);//filling fileStat with statistics of file
 
 return (uint32_t)fileStat.st_size;//returning only the size of file from stat stru    ct
}



void createConnection(int * sockfd, struct sockaddr_in * sockaddr, char *port, char * serverIp)
{
	*sockfd = SOCKET(AF_INET, SOCK_STREAM,0);
	if(*sockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
		exit(0);
	}
	
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons(atoi(port));
	sockaddr->sin_addr.s_addr = inet_addr(serverIp);	
	memset(&(sockaddr->sin_zero),'\0',8);
	if(BIND(*sockfd,(struct sockaddr *)sockaddr,sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "%s failed becuase %s\n", "failed to bind", strerror(errno));
		close(*sockfd);
		exit(0);
	}
	return;
}


int main(int args, char * argv[])
{
	if (args != 4)
	{
		fprintf(stderr,"%s %d %s\n" , "please give three aurguments,",args - 1,"were entered, the aurguments are remote-ip, remote-port, local-file-to-transfer");

	}
	
	FILE* transFile = fopen(argv[3],"r");//trying to open local-file-to-transfer  in read mode

	if(transFile == NULL)//Checking if input file was opened correctly
 	{
		fprintf(stdout,"%s","please enter a valid path to an existing file for the third argument\n");
		exit(0);//if not opened correctly, exit program
	}
 	uint32_t fsize = fileSize(argv[3]);	
	int sockfd;
	struct sockaddr_in sockaddr;		
	createConnection(&sockfd, &sockaddr, argv[2], argv[1]);
	sendFile(&sockfd,transFile,fsize,argv[3]);
	fprintf(stdout, "%s %lu %d \n", "ended normally", sizeof(uint32_t), fsize);
}
