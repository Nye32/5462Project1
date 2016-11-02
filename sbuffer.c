

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


char buffer[64000];
int dstart = 0;
int dend = 0;
int isempty = 1;


struct cirNode
{
	struct cirNode * next;
	int firstSeq;
	int lastSeq;
	int data;
	int length;
	int filled;
};

struct cirNode * head;


struct cirNode * newCirNode()
{
	struct cirNode * cir = (struct cirNode *) malloc(sizeof(struct cirNode));
	cir->next = NULL;
	cir->data = 0;
	cir->length = 0;
	cir->filled = 0;
	return cir;
}


void initHead()
{
	head->next = newCirNode();
	head->next->data = 0;
	head->next->length = 64000;
	head->next->filled = 0;
	head->next->firstSeq = 1;
	head->next->lastSeq = 64;
}

void  freeCirNode(struct cirNode * cir) 
{
	free(cir);
}
int validbuffer(struct cirNode* cir)
{

}

void addData(int seqnum, int size ,char * data)
{		
	struct cirNode * current = head->next;
	while(current != NULL)
	{
		if(!(current->filled))
		{
			if(current->lastSeq >= seqnum && current->firstSeq <= seqnum)
			{
				if(seqnum == current->firstSeq)
				{

				}
				int x = (current->firstSeq - seqnum) * 1000 + current->data;
				if(size + x > 64000)
				{
					memcpy(buffer+x,data,64000-x);
					memcpy(buffer, data, size - (64000 - x));
				}

				
			}
		}

	}
}

















/*
//___________________________
//| Andrew Lee				|
//| CSE5462	Project 1		|
//| 10-17-16 sbuffer.c		|
//|_________________________|

// Handles contiguous buffer implementation
// Complete and utter garbage

// Required Libraries
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


// Global Variables
struct listnode *head;
struct listnode *tail;
struct listnode *current;
char *buffer;
char *dataStart;
char *dataEnd;

// Header of each packet
struct packet {
	char *data;
	int startSeqNum;
	int endSeqNum;
	int size;
};

// listnode
struct listnode {
	int hasData;
	struct packet packet;
	struct listnode *prev;
	struct listnode *next;
};

// Initialize List
void initializeList() {
	// blank packet
	struct packet p;
	p.data = NULL;
	p.size = 64000;

	head = malloc(sizeof(struct listnode));
	head->hasData = 0;
	head->packet = p;
	head->prev = NULL;
	head->next = NULL;
	current = head;
	tail = current;
	current = head;
}

// Initialize
void initialize() {
	//initializeList();

	// Initialize Buffer
	buffer = (char *)malloc(64000);
	bzero(buffer,64000);
}

// returns 1 if list contains byteseqnum, 0 otherwise
int contains(int byteSeqNum) {
	current = head;
	while (current->next != head) {
		if (current->packet.startSeqNum = byteSeqNum) {
			return 1;
		}
	}
	return 0;
}

// Writes data to buffer -- returns pointer to buffer location (aka start)
char *writeData(int size, char *start, char *data) {
	// Check for wraparound
	if (start + size > &buffer[64000]) {
		int endsize = &buffer[64000] - start;
		// fill out end of buffer
		memcpy(start, data, endsize);
		// write at beginning
		memcpy(&buffer[0], data, size - endsize);
	// No Wraparound
	} else {
		memcpy(start, data, size);
	}
	return(start);
}

// Goes through list and merges when possible
void mergeList() {
	current = head;
	while (current->next != head) {
		if (current->packet.endSeqNum == current->next->packet.startSeqNum
			&& current->hasData == current->next->hasData) {
			// Merge
			struct listnode *temp = current->next;
			current->packet.size = current->packet.size + current->next->packet.size;
			current->next = current->next->next;
			current->packet.endSeqNum = current->next->packet.endSeqNum;
			free(temp);
		}
		current = current->next;
	}
}

// Inserts value at appropriate position
int insertValue(int byteSeqNum, int size, char *data) {
	current = head;
	printf("INSERTING %d\n", byteSeqNum);
	
	// If no list exists, create one
	if (current == NULL) {
		head = malloc(sizeof(struct listnode));
		head->packet.data = writeData(size, buffer, data);
		head->packet.startSeqNum = byteSeqNum;
		head->packet.endSeqNum = byteSeqNum;
		head->packet.size = size;
		head->hasData = 1;
		current = head;
		perror("hitashi");
		// Create Node for rest of data
		struct listnode *node = (struct listnode*)malloc(sizeof(struct listnode));
		node->packet.size = 64000 - size;
		node->hasData = 0;
		node->packet.data = current->packet.data + current->packet.size;
		node->packet.startSeqNum = head->packet.endSeqNum;
		node->packet.endSeqNum = -1;
		node->prev = head;
		node->next = head;
		head->prev = node;
		head->next = node;
		tail = node;

	} else {
		// Check to see if inserting adjacent to another node
		current = head;
		// Create Node
		struct listnode *node = (struct listnode*)malloc(sizeof(struct listnode));
		node->hasData = 1;
		node->packet.size = size;
		node->packet.startSeqNum = byteSeqNum;
		node->packet.endSeqNum = byteSeqNum;
		while (current->next != head) {
			// Node is adjacent to previous
			if (current->packet.startSeqNum == byteSeqNum && current->hasData == 0) {
				// Insert node
				node->prev = current->prev;
				// Check to see if hole is filled
				if (current->packet.size == size) {
					node->next = current->next;
				} else {
					node->next = current;
				}
				node->packet.data = writeData(size, node->prev->packet.data + node->prev->packet.size, data);
				current->prev = node;
				current->packet.size = current->packet.size - size;
				break;
			// Node is part of a hole
			} else if (current->packet.startSeqNum < byteSeqNum 
				&& current->hasData == 0 
				&& current->packet.size > size) {
				// Create split node
				struct listnode *split = (struct listnode*)malloc(sizeof(struct listnode));
				split->hasData = 0;
				split->packet.size = current->packet.size - (((byteSeqNum - current->prev->packet.endSeqNum)*1000) + 1000);
				split->packet.startSeqNum = byteSeqNum;
				split->packet.endSeqNum = current->packet.endSeqNum;

				// Insert nodes
				split->prev = node;
				split->next = current->next;
				node->prev = current;
				node->next = split;
				current->next = node;
				current->packet.endSeqNum = byteSeqNum;
				current->packet.size = ((byteSeqNum - current->prev->packet.endSeqNum)*1000);
				node->packet.data = writeData(size, node->prev->packet.data + node->prev->packet.size, data);

				break;
			// Node is back end of hole
			} else if (current->packet.startSeqNum == byteSeqNum && current->hasData == 1) {
				node->prev = current->prev;
				node->next = current;
				current->prev->next = node;
				current->prev->packet.size = current->prev->packet.size - 1000;
				current->prev = node;
				node->packet.data = writeData(size, node->next->packet.data - size, data);
				break;
			}
			current = current->next;
		}
		// Ensure node was inserted
		if (contains(byteSeqNum) == 0) {
			printf("INSERTVALUE: Did not insert for some stupid reason.\n");
			return -1;
		}
	}
	// Merge if Possible
	mergeList();
	return 0;
}

// Removes head and modifies list
void removeHead() {
	current = head;
	current->prev->next = head->next;
	current->next->prev = head->prev;
	struct listnode *temp = head;
	current = current->next;
	head = temp->next;
	free(temp);
}

// gets size of first node (if it contains data)
int getSize() {
	if (head == NULL ||  head->hasData == 0) {
		return -1;
	} else {
		return head->packet.size;
	}
}

// Checks and gets data
void getData(int size, char * temp) {
	// Check to see if there is data available
	if (getSize() == -1 || getSize() < size) {
		printf("GETDATA: Unable to get data.\n");
		temp = NULL;
		return;
	} else {
		// wraparound
		if (head->packet.data + size > &buffer[64000]) {
			int endsize = &buffer[64000] - head->packet.data;
			// write from end of buffer
			memcpy(temp, head->packet.data, endsize);
			// write from beginning of buffer
			memcpy(temp + endsize, &buffer[0], size - endsize);
		// No Wraparound
		} else {
			memcpy(temp, head->packet.data, size);
		}
		removeHead();
		mergeList();
		return;
	}
}

// free data
void freeBuffer(char *buffer) {
	free(buffer);
}

// Prints the current Linked List
int printList() {
	int count = 0;
	current = head;
	if (current == NULL) {
		printf("No List Exists.\n");
	} else {
		printf("Windowed List List\n");
		printf("________________\n");
		while (current->next != head) {
			printf("%d: %d (%d, %d) %d\n",count, current->hasData, current->packet.startSeqNum, current->packet.endSeqNum, current->packet.size);
			current = current->next;
			count++;
		}
		printf("%d: %d (%d, %d)\n",count, current->hasData, current->packet.startSeqNum, current->packet.endSeqNum);
	}
	return 0;
}
*/
