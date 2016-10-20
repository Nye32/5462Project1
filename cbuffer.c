



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
#include "cbuffer.h"

struct packNode
{
	int seqnum;
	int data;
	int size;
	int ack; 
	struct packNode * next;
};


char cirbuffer[64000];
struct packNode * head;
int dstart = 0;
int dend = 0;
int windowend = 0;
int packseq = 1;
int isEmpty = 1;



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

void initHead()
{
	head = newPackNode(0,0,0);
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
	if(abs(spaceLeft()) >= 0 && abs(spaceLeft()) > size)
	{
		if((64000 - dend) > size)
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
				freePackNode(current);
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
	if((64000-spaceLeft()) >= 2000)
	{
		windowend = (dstart + 2000) % 64000;
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

	current = head->next;
	while(current != NULL)
	{
	//	fprintf(stderr,"\nseqnum = %d, data = %d, size = %d\n",current->seqnum,current->data,current->size);
		current = current->next;
	}



}



int requestBSN()
{
	struct packNode * current = head->next;
	while(current != NULL)
	{
		if(current->ack > 0)
			current = current->next;
		else
		{
			int num = current->seqnum;
			current->ack = 1;
			return num;
		}
	}
	return -1;
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


int recvACK(int num)
{

	struct packNode * current = head->next;
	while(current!=NULL)
	{
		if(current->seqnum == num)
		{
			current->ack = 2;
			return 1; 
		}
	}
	return 0;

}









