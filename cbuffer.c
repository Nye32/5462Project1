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
#include <time.h>
#include <math.h>
#include <inttypes.h>
#include "tcpapi.h"
#include "cbuffer.h"
#include "crc32.h"

// TIMER FUNCTIONS==============================
int timersockfd;
struct sockaddr_in cli_addr;
struct sockaddr_in expire;

// Establishes connection with host - Copied from Lab02
void createTimerConnection(int * timersockfd, struct sockaddr_in * cli_addr)
{
	//*timersockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	*timersockfd = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(*timersockfd < 0)
	{
		fprintf(stdout, "%s\n", "socket could not be made");
		exit(0);
	}
	
	cli_addr->sin_family = AF_INET;
	cli_addr->sin_port = htons(atoi("3132"));
	cli_addr->sin_addr.s_addr = inet_addr("127.0.0.1");	
	memset(&(cli_addr->sin_zero),'\0',8);

	// Expiration use
	expire.sin_family = AF_INET;
	expire.sin_port = htons(atoi("3231"));
	expire.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(&(expire.sin_zero), '\0',8);
	if(BIND(*timersockfd, (struct sockaddr *)&expire, sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"%s\n","couldn't bind socket");
		exit(0);
	}

	// Set SEND address
	setSendAddress(*(struct sockaddr *)cli_addr);

	return;
}

// Starts timer in timerprocess
void starttimer(double time, uint32_t byteSeqNum) {
	// convert double to two ints
	int itemp = time;
	double dtemp = time - itemp;
	uint32_t ltime = htonl(itemp);
	itemp = dtemp * 1000;
	uint32_t ldec = htonl(itemp);

	uint32_t flag = htonl(1);
	uint32_t lbyte = htonl(byteSeqNum);
	int isize = sizeof(uint32_t);
	int dsize = sizeof(double);
	char * buf = (char *) malloc(4*isize);
	bzero(buf,(4*isize));
	memcpy(buf, &flag,isize);
	memcpy(buf+isize,&lbyte,isize);
	memcpy(buf+(2*isize), &ltime,isize);
	memcpy(buf+(3*isize), &ldec,isize);
	//sendto(timersockfd,buf,(4*isize),0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	SEND(timersockfd,buf,(4*isize),0);
	printf("SENT\nFlag: %d\nByte: %d\nTime: %d.%d\n", ntohl(flag), ntohl(lbyte), ntohl(ltime), ntohl(ldec));
}

// Cancels timer in timerprocess
void canceltimer(uint32_t byteSeqNum) {
	uint32_t flag = htonl(2);
	uint32_t lbyte = htonl(byteSeqNum);
	int isize = sizeof(uint32_t);
	char * buf = (char *) malloc(2*isize);
	bzero(buf,2*isize);
	memcpy(buf, &flag,isize);
	memcpy(buf+isize, &lbyte,isize);
	//sendto(timersockfd,buf,(4*isize),0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	SEND(timersockfd, buf, (4*isize), 0);
	printf("SENT\nFlag: %d\nByte: %d\n", ntohl(flag), ntohl(lbyte));
}

// Sends quit flag to timer
void timerquit() {
	uint32_t flag = htonl(3);
	int isize = sizeof(uint32_t);
	int dsize = sizeof(double);
	char * buf = (char *) malloc(2*isize);
	bzero(buf,2*isize);
	memcpy(buf, &flag,isize);
	//sendto(timersockfd,buf,(4*isize),0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	SEND(timersockfd, buf, (4*isize), 0);
	printf("SENT\nFlag: %d\n", ntohl(flag));
}

// END TIMER FUNCTIONS==========================

struct packNode
{
	int seqnum;
	int data;
	int size;
	int ack; 
	struct packNode * next;
};


struct trollSock
{
	struct sockaddr_in remote;
	uint32_t crc32;
	uint32_t packnum;
	char buffer[1000];// = malloc(1000);
}pack;

struct rttPack
{
	int bsn;
	int on;
	struct timespec timestart;
};


char cirbuffer[64000];
struct rttPack packtimes[20];
struct packNode * head;
int dstart = 0;
int dend = 0;
int windowend = 0;
int packseq = 1;
int isEmpty = 1;
double SRTT = 0;
double RTTVAR = 0;
double RTO = 1000;


void rttPackSetup(struct rttPack rttpack)
{
	rttpack.bsn = 0;
	rttpack.on = 0;
}

//creating the socket descritor and the sockaddrs for forwarding the packet
int remoteSockfd;
struct sockaddr_in trolladdr;
//creating the connections required for UDP SEND




//for the necessary connections required for sending the file and receiving the file
void makeConnections(struct sockaddr_in * trolladdr,char * serverIp)
{


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


struct packNode * newPackNode(int dat, int siz, int seq)
{
	struct packNode* pack = (struct packNode *) malloc(sizeof(struct packNode));
	pack->seqnum = seq;
	pack->size = siz;
	pack->data = dat;
	pack->ack = 0;
	pack->next= NULL;
	return pack;
}

void initialize(char * serverIp, int remoteSock)
{
	head = newPackNode(0,0,0);
	remoteSockfd = remoteSock;
	makeConnections(&trolladdr, serverIp);
	
	for(int i = 0; i<20; i++)
	{
		rttPackSetup(packtimes[i]);
	}

}


void freePackNode(struct packNode * pack)
{
	free(pack);
}


int min(int a, int b)
{
	if(a < b)
	{
		return a;
	}
	return b;
}

int spaceLeft()
{
	if(dend < dstart)
	{
		isEmpty = 0;
		return (dstart - dend);
	}
	else if(dstart < dend)
	{
		isEmpty = 0;
		return(64000 - (dend - dstart));
	}


	if(isEmpty == 1)
		return 64000;
	else
	{
		return 0;
	}
}


int isFull()
{
	if(spaceLeft() == 0)
	{
		return 1;
	}
	return 0;
}


int addData(char * data, int size)
{
	int retval = 0;
	if(abs(spaceLeft()) >= size)
	{
		if((64000 - dend) >= size)
		{
			memcpy(cirbuffer+dend,data, size);
			dend += size;
		}
		else
		{
			memcpy(cirbuffer+dend,data,64000-dend);
			memcpy(cirbuffer, data + (64000-dend),size - (64000 - dend));
			dend = (size - (64000 - dend));
		}
		retval = 1;
		isEmpty = 0;
	}
	else
	{
		retval = 0;
	}
	checkWindow();
	return retval;
}



void checkWindow()
{
	struct packNode * current = head->next;
	while(current != NULL)
	{
		if(current->ack == 2)
		{
			if(current->next == NULL)
			{
				dstart = (dstart + current->size) % 64000;
				struct packNode * cur = current;
				freePackNode(cur);
				current = NULL;
				head->next = NULL;
				isEmpty = 1;
			}
			else
			{
				dstart = (dstart + current->size) % 64000;
				struct packNode * cur = current;
				current = current->next;
				freePackNode(cur);
				head->next = current;
			}
		}
		else
		{
			break;
		}
	}


	current = head->next; 
	if((64000-spaceLeft()) >= 20000)
	{
		windowend = (dstart + 20000) % 64000;
	}
	else
	{
		windowend = ((64000-spaceLeft()) + dstart) % 64000;
	}

	int disttowindow = 0;
	if(dstart < windowend)
	{
		disttowindow = windowend - dstart;
	}
	else if(windowend < dstart)
	{
		disttowindow = (64000 - dstart) + windowend;
	}
	if(disttowindow > 0)
	{
		if(current == NULL)
		{
			int mini = min(1000,disttowindow);
			current = newPackNode(dstart, mini,packseq++);
			head->next = current;
			disttowindow -= mini;
		}
		else
		{
			while(current->next != NULL)
			{
				disttowindow -= current->size;
				current = current->next;
				
			}
			disttowindow -= current->size;
		}


		while(disttowindow > 0)
		{
			int mini = min(1000,disttowindow);
			current->next = newPackNode((current->data + current->size) % 64000, mini,packseq++);
			disttowindow -= mini;
			current = current->next;
		}
	}
	current = head->next;
	while(current != NULL)
	{
		fprintf(stderr,"\nseqnum = %d, data = %d, size = %d\n",current->seqnum,current->data,current->size);
		current = current->next;
	}
	fprintf(stderr,"%d------------------------%d\n",dstart, dend);



}



void sendWindow()
{
	struct packNode * current = head->next;
	/*int extwindow = 0;
	if(windowend < dstart)
		extwindow  = 64000 + windowend;
	else
		extwindow = windowend;
	while(current != NULL && (current->data + current->size) < extwindow)
	{*/
	while(current != NULL)
	{
		if(current->ack > 0)
			current = current->next;
		else
		{
			int bsn = current->seqnum;
			int size = requestSize(bsn);
			char data[size];
			requestData(bsn, data);

			memcpy(pack.buffer, data, size);
	
			//do the crc checksumming
			uint32_t crcpoly = 0x04C11DB7;
			pack.crc32 = crc32(crcpoly,pack.buffer,size);
		
			//printing checksum
			fprintf(stderr,"checksum = %d\n",pack.crc32);

			memcpy(&(pack.packnum), &bsn, sizeof(uint32_t));
			fprintf(stderr,"packetnumber = %d\n", bsn);

			//Sending the changed packet forward to troll
			setSendAddress(*(struct sockaddr *) &trolladdr);
			
			int i = 0;
			while(packtimes[i].on != 0)
			{
				i++;
			}

			clock_gettime(CLOCK_MONOTONIC, &(packtimes[i].timestart));	
			if(SEND(remoteSockfd, (char *)&pack, size+sizeof(struct trollSock)-1000, 0) < 0)
			{
				fprintf(stderr, "%s %s \n", "couldn't send...quiting...", strerror(errno));
				exit(0);
			}
			
			packtimes[i].on = 1;
			packtimes[i].bsn = bsn;
			current->ack = 1;
		}
	}
}


int requestSize(int num)
{
	struct packNode * current = head->next;
	while(current != NULL)
	{
		if(current->seqnum == num)
		{
			current->ack = 1;
			return current->size;
		}
		current = current->next;
	}
	return 0;
}


void requestData(int num, char * d)
{	struct packNode * current = head->next;
 	while(current != NULL)
	{
 		if(current->seqnum == num)
 		{
 			current->ack = 1;
			if(current->size + current->data > 64000)
			{
				memcpy(d,(cirbuffer+(current->data)), (64000-(current->data)));
				memcpy(d + (64000-(current->data)),cirbuffer, current->size - (64000-current->data));
			}
			else
			{
				memcpy(d, cirbuffer+(current->data), current->size);
			}
			return;
 		}
		current = current->next;
 	}
	d = NULL;
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


int recvACK(int num)
{
	struct timespec stop, start;
	
	clock_gettime(CLOCK_MONOTONIC, &stop);
	int i = 0;
	while(packtimes[i].bsn != num)
		i++;

	start = packtimes[i].timestart;	

	double  delta = ((stop.tv_sec - start.tv_sec) * 1000.0) + ((stop.tv_nsec - start.tv_nsec) / 1000000.0);
	fprintf(stderr,"delta = %f\n",delta);
	timeComps(delta);
	
	packtimes[i].on = 0;

	struct packNode * current = head->next;
	while(current!=NULL)
	{
		if(current->seqnum == num)
		{
			current->ack = 2;
			checkWindow();
			return 1; 
		}
		else
			current = current->next;
	}
	return 0;

}




