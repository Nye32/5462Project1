

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
int dstart = 1;
int dend = 1;
int isempty = 1;


struct cirNode
{
	struct cirNode * next;
	int firstSeq;
	int lastSeq;
	int data;
	int size;
	int filled;
};

struct cirNode * head;


struct cirNode * newCirNode()
{
	struct cirNode * cir = (struct cirNode *) malloc(sizeof(struct cirNode));
	cir->next = NULL;
	cir->data = 0;
	cir->size = 0;
	cir->filled = 0;
	return cir;
}

void initHead()
{
	head = newCirNode();
	head->next = head;
	head->data = 0;
	head->size = 64000;
	head->filled = 0;
	head->firstSeq = 1;
	head->lastSeq = 64;
}

void initialize() {
	initHead();
}

void  freeCirNode(struct cirNode * cir) 
{
	free(cir);
}
int validbuffer(struct cirNode* cir)
{

}

// Prints list (for troubleshooting)
void printList() {
	struct cirNode * current = head;
	if (current == NULL) {
		printf("No List.\n");
	} else {
		printf("dstart:\t%d\n", dstart);
		printf("dend:\t%d\n", dend);
		while (current->next != head) {
			printf("_________________\n");
			printf("Filled:\t\t%d\n", current->filled);
			printf("FirstSeq:\t%d\n", current->firstSeq);
			printf("LastSeq:\t%d\n", current->lastSeq);
			printf("Data:\t\t%d\n", current->data);
			printf("Size:\t\t%d\n", current->size);
			printf("NextSeq:\t%d\n", current->next->firstSeq);
			printf("_________________\n");
			current = current->next;
		}
		printf("_________________\n");
			printf("Filled:\t\t%d\n", current->filled);
			printf("FirstSeq:\t%d\n", current->firstSeq);
			printf("LastSeq:\t%d\n", current->lastSeq);
			printf("Data:\t\t%d\n", current->data);
			printf("Size:\t\t%d\n", current->size);
			printf("NextSeq:\t%d (HEAD)\n", current->next->firstSeq);
			printf("_________________\n");
	}
}

// Goes through list and merges when possible
void mergeList() {
	struct cirNode * current = head;
	while (current->next != head && current!= NULL) {
		while(current->lastSeq == (current->next->firstSeq - 1)
			&& current->filled == current->next->filled) {
			// Merge
			printf("Merging %d & %d.\n", current->lastSeq, current->next->firstSeq);
			current->lastSeq = current->next->lastSeq;
			current->size = current->size + current->next->size;
			current->next = current->next->next;
		}
		while (current->filled == 0 && current->next->filled == 0
			&& dstart != current->firstSeq && dstart != current->next->firstSeq) {
			// Merge
			printf("Merging %d & %d.\n", current->lastSeq, current->next->firstSeq);
			current->lastSeq = 2147483647; // Max int value
			current->size = current->size + current->next->size;
			current->next = current->next->next;
		}

		current =  current->next;
	}
	
	// reset size if only one empty node
	/*if (head->next == head && head->filled ==0) {
        head->size = 64000;
        head->data = 0;
    }*/
    
}


int addData(int seqnum, int size ,char * data)
{	
	// Merge List to ensure it is proper
	mergeList();
    struct cirNode * n  = head;
    int e = 1;
    while(n->next != head)
    {
        e++;
        n = n->next;
    }
    fprintf(stderr,"nums= %d",e);
	struct cirNode * current = head;
	do{
		// Check for entry that already exists
		if (current->firstSeq == seqnum && current->filled == 1) {
			// Already have data!
			// TODO - Return something
			printf("Duplicate seqnum");
			return 1;
		} else if (seqnum > (dstart+63)) {
			perror("Seqnum out of range!");
			break;
		}
		// current is empty
		if (current->filled == 0) {
			// data fills beginning of hole
			if (current->firstSeq == seqnum) {
				// data will not fill hole
				if (size < current->size) {
					// create new node to insert
					struct cirNode * node = newCirNode();
					node->size = current->size - size;
					node->data = current->data + size;
					node->firstSeq = seqnum + 1;
					node->lastSeq = current->lastSeq;

					// Update current
					current->size = size;
					current->filled = 1;

					current->lastSeq = seqnum;
					node->next = current->next;
					current->next = node;

				// data fills hole
				} else if (size == current->size) {
					// Update Current
					current->filled = 1;
					current->firstSeq = seqnum;
					current->lastSeq = seqnum;
				}
			// data fills middle of a hole
			} else if (current->firstSeq < seqnum
				&& current->lastSeq > seqnum) {
				// create two new nodes
				struct cirNode * nodeA = newCirNode(); // Contains new data
				struct cirNode * nodeB = newCirNode(); // Contains remaining hole

				// Set filled
				nodeA->filled = 1;
				nodeB->filled = 0;

				// Set seq numbers
				nodeA->firstSeq = seqnum;
				nodeA->lastSeq = seqnum;
				nodeB->firstSeq = seqnum + 1;
				nodeB->lastSeq = current->lastSeq;
				current->lastSeq = seqnum - 1;

				// Set Sizes (ASSUMING PACKET SIZE OF 1000)
				nodeA->size = size;
				int temp = current->size;
				current->size = ((current->lastSeq - current->firstSeq + 1) * 1000);
				nodeB->size = temp - (current->size + nodeA->size);

				// set data values
				// nodeA wraparound
				if ((current->data + current->size) > 64000) {
					nodeA->data =(current->data + current->size) - 64000;
				// nodeA nonwrap
				} else {
					nodeA->data = current->data + current->size;
				}
				// nodeB wraparound
				if ((nodeA->data + nodeA->size) > 64000) {
					nodeB->data =(nodeA->data + nodeA->size) - 64000;
				// nodeB nonwrap
				} else {
					nodeB->data = nodeA->data + nodeA->size;
				}

				// Set Order
				nodeB->next = current->next;
				current->next = nodeA;
				nodeA->next = nodeB;

				current = nodeA;

			// data fills end of a hole
			} else if (current->firstSeq < seqnum
				&& current->lastSeq == seqnum) {

				// create new node to insert
				struct cirNode * node = newCirNode();
				node->size = size;
				node->firstSeq = seqnum;
				node->lastSeq = seqnum;
				node->next = current->next;
				node->filled = 1;

				// set data value
				// node wraparound
				if ((current->data + current->size - size)> 64000) {
					node->data = (current->data + current->size - size) - 64000;
				// no wrap
				} else {
					node->data = current->data + current->size - size;
				}
				// Update Current
				current->next = node;
				current->size = current->size - size;
				current->lastSeq = seqnum - 1;

				current = node;
			} 

			// write data to buffer if node was created
			if (current->filled == 1) {
				// wraparound
				if ((current->data + current->size) > 64000) {
					memcpy(buffer + current->data, data, 64000 - current->data);
					memcpy(buffer, data + (64000 - current->data), current->size - (64000 - current->data));
				// no wrap
				} else {
					memcpy(buffer + current->data, data, current->size);
				}

				// Update dvalues
				if (seqnum > dend) {
					dend = seqnum;
				}

				// Merge
				mergeList();

				// End
				return 1;
			}
		}

		// Move Current
		current = current->next;
	}
	while(current != head);




	printf("Unable to Insert data!\n");
	return 0;
}
// gets size of first node (if it contains data)
int getSize() {
	// check to see if head has data
	if (head->filled == 0) {
		return 0;
	} 
	return head->size;
}


// sets *d equal to head data and free's node... returns size
int requestData(int size, char * d)
{	struct cirNode * current = head;

	if (current->filled == 1 && current->firstSeq == dstart) {
		// Wants more or equal data of current
		if (current->size <= size) {
			int returnSize = current->size;
			// check wrapround
			if(current->size + current->data > 64000)
			{
				memcpy(d, (buffer + current->data), (64000-(current->data)));
				memcpy(d + (64000-(current->data)), buffer, current->size - (64000-current->data));
			}
			// no wraparound
			else
			{
				memcpy(d, buffer+(current->data), current->size);
			}

			// set filled to 0;
			current->filled = 0;

			// reset head and dvalue
			dstart = current->lastSeq + 1;
			head = head->next;

			// merge
			mergeList();
			return(returnSize);

		// Current has more data then what is being requested
		} else {
			// check wrapround
			if(current->size + current->data > 64000)
			{
				memcpy(d, (buffer + current->data), (64000-(current->data)));
				memcpy(d + (64000-(current->data)), buffer, size - (64000-current->data));
			}
			// no wraparound
			else
			{
				memcpy(d, buffer+(current->data), size);
			}

			// Adjust data and size
			current->data = current->data + size;
			current->size = current->size - size;

			return(size);
		}
 	} else {
		d = NULL;
	}
 	return 0;
}
