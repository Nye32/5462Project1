//___________________________
//| Andrew Lee				|
//| CSE5462	Project 1		|
//| 09-26-16 timerprocess	|
//|_________________________|

// Handles timer implementation using a delta-list.

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

#define BUFSIZE 1000

// Global Variables
int port;
int msgsock;
struct sockaddr_in sin_addr;
struct listnode *head;
struct listnode *tail;
struct listnode *current;

// listnode
struct listnode {
	double dtime;
	int port;
	int byteSeqNum;
	struct listnode *prev;
	struct listnode *next;
};

// Inserts value at appropriate position
int insertValue(double dtime, int port, uint32_t byteSeqNum) {
	double elapse = 0.0;
	current = head;
	printf("INSERTING %d\n", byteSeqNum);
	
	// If no list exists, create one
	if (current == NULL) {
		head = malloc(sizeof(struct listnode));
		head->dtime = dtime;
		head->port = port;
		head->byteSeqNum = byteSeqNum;
		head->prev = NULL;
		head->next = NULL;
		current = head;
		tail = head;
	} else {
		// Special Cases
		if (dtime == current->dtime) {
			elapse = dtime;
		} else if (current->next != NULL) {
			// Traverse List to get to value
			while((elapse + current->dtime) < dtime) {
				elapse += current->dtime;
				// Last Node?
				if (current->next == NULL) {
					break;
				} else {
					current = current->next;
				}
			}
		}
		
		// Create and insert node
		struct listnode *node = (struct listnode*)malloc(sizeof(struct listnode));
		node->dtime = (dtime-elapse);
		node->port = port;
		node->byteSeqNum = byteSeqNum;
		// Replace Head
		if (dtime < head->dtime) {
			node->prev = NULL;
			node->next = current;
			current->prev = node;
			head = node;
		// Non Head/Tail
		} else if(current->next !=  NULL) {
			node->next = current->next;
			current->next->prev = node;
			node->prev = current;
			current->next = node;
		// Current is on tail
		} else {
			// Replace tail
			if (elapse + current->dtime <= dtime) {
				if (elapse == 0.0) {
					node->dtime = dtime - current->dtime;
				}
				current->next = node;
				node->next = NULL;
				node->prev = current;
				tail = node;
			// Insert prior to tail
			} else {
				current->prev->next = node;
				node->prev = current->prev;
				current->prev = node;
				node->next = current;
			}
		}
		
		// Update following dtime value
		current = node;
		if (current->next != NULL) {
			current->next->dtime = (current->next->dtime - current->dtime);
		}
	}
	return 0;
}

// Removes selected byteSeqNum and modifieds list
int removeValue(uint32_t byteSeqNum) {
	current = head;
	printf("REMOVING %d\n", byteSeqNum);
	// Determine if empty list
	if (current == NULL) {
		printf("REMOVE ERROR: Cannot remove from empty list.\n");
		return (0);
	}
	// Traverse List to get to value
	while(current->byteSeqNum != byteSeqNum) {
		if (current->next == NULL) {
			printf("REMOVE ERROR: Invalid byteSeqNum %d.\n", byteSeqNum);
			return(-1);
		} else {
			current = current->next;
		}
	}
	// If it is only value in list
	if (current->next == NULL && current->prev == NULL) {
		head = NULL;
		tail = NULL;
		free(current);
		current = NULL;
		return (0);
	// If it is the last value
	} else if (current->next == NULL) {
		tail = current->prev;
		tail->next = NULL;
		free(current);
		current = tail;
		return (0);
	// If it is the first value
	} else if (current->prev == NULL) {
		head = current->next;
		current->next->prev = current->prev;
	} else {
		current->prev->next = current->next;
		current->next->prev = current->prev;
	}

	// Update list and remove value
	int dtime = current->dtime;
	struct listnode *temp = current;
	current = current->next;
	free(temp);

	// Update following dtime value
	current->dtime = ((current->dtime)+dtime);
	return 0;
}

void expiration(int byteSeqNum) {
	printf("Expiration Notice: %d.\n", byteSeqNum);

	// Construct Expiration Packet
	int temp = htonl(byteSeqNum);
	char * buf = (char *) malloc(sizeof(int));
	bzero(buf,sizeof(int));
	memcpy(buf, &temp, sizeof(int));
	sendto(msgsock,buf,sizeof(int),0, (struct sockaddr *)&sin_addr, sizeof(sin_addr));
}

// Prints the current Linked List
int printList() {
	int count = 0;
	current = head;
	if (current == NULL) {
		printf("No List Exists.\n");
	} else {
		printf("Delta Timer List\n");
		printf("________________\n");
		while (current->next != NULL) {
			printf("%2d: (%2f, %2d)\n",count, current->dtime, current->byteSeqNum);
			current = current->next;
			count++;
		}
		printf("%2d: (%2f, %2d)\n\n\n",count, current->dtime, current->byteSeqNum);
	}
	return 0;
}

// Decrements time based on prev value
double decrement(double prev) {
	struct timespec monotime;
	clock_gettime(CLOCK_MONOTONIC, &monotime);
	time_t nows = monotime.tv_sec;
	double nowms = monotime.tv_nsec / 1000000;
	double now = nows + (nowms / 1000);
	double dif = now - prev;

	// Troubleshooting Printouts
	//printf("prev: %f.\n", prev);
	//printf("now: %f.\n", now);
	//printf("elapsed: %f.\n", dif);

	if (head != NULL) {
		head->dtime = head->dtime - dif;
		if (head->dtime == 0.0) {
			removeValue(head->byteSeqNum);
		// Time difference is greater than delay of head
		} else if (head->dtime < 0.0) {
			while (head->dtime <= 0.0) {
				// Send Expiration notice
				expiration(head->byteSeqNum);
				if (head->next == NULL) {
					removeValue(head->byteSeqNum);
					printList();
					break;
				} else {
					head->next->dtime = head->next->dtime + head->dtime;
					removeValue(head->byteSeqNum);
				}
			}
		}
		printList();
		return(now);
	}
	return(now);
}


int main(int argc, char * argv[]) {
	// Variables
	int rval = 0;
	char buf[BUFSIZE];
	
	// Ensure Proper Argument Usage
	if (argc != 2) {
		printf("ERROR: Incorrect number of arguements.");
		printf(" Please run using the following format...\n");
		printf("tp <local-port>\n\n");
		exit(1);
	}
	port = atoi(argv[1]);
	
	// Wait for client connection
	if ((msgsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("Error: Unable to Open Datagram Socket.");
		exit(1);
	}
	
	// Construct sent socket
	sin_addr.sin_family = AF_INET;
	sin_addr.sin_addr.s_addr = INADDR_ANY;
	sin_addr.sin_port = htons(port);
		

	// Bind Socket
	if (bind(msgsock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("Error: Unable to establish UDP.");
		exit(1);
	}
	printf("timerprocess Initialized. Awaiting connection...\n");

	// Listen for commands
	struct timespec monotime;
	clock_gettime(CLOCK_MONOTONIC, &monotime);
	time_t prevs = monotime.tv_sec;
	double prevms = monotime.tv_nsec / 1000000;
	double prev = prevs + (prevms / 1000);

	int addr_len = sizeof(sin_addr);
	fd_set readfds;
	struct timeval tv;
	while(1) {

		// Set Select sleep (either 10000usec or wait time of head node)
		tv.tv_sec = 0;
		if (head != NULL) {
			tv.tv_usec = head->dtime * 1000000;
		} else {
			tv.tv_usec = 10000;
		}

		FD_ZERO(&readfds);
		FD_SET(msgsock, &readfds);
		if (select(FD_SETSIZE, &readfds, NULL, NULL, &tv) < 0) {
			perror("Select error");
		} else if (FD_ISSET(msgsock, &readfds)) {
			// Clear Buffer (Set it to 0)
			bzero(buf, sizeof(buf));
			//recv(msgsock, buf, 3*sizeof(int), 0);
			rval = recvfrom(msgsock, buf, 2*sizeof(int)+sizeof(double), 0, (struct sockaddr *)&sin_addr, &addr_len);
			rval = rval - 3*sizeof(int);
			// Determine values 
			int flag = 0;
			uint32_t byte = 0;
			double time = 0.0;
			uint32_t itime = 0;
			uint32_t dtime = 0;
			char temp[4];
			int i = 0;
			int c = 0;
			for (i = 0; i < 16; i++) {
				temp[c] = buf[i];
				c++;
				if (i == 3) {
					flag = ntohl(*((uint32_t *)(temp)));
					c = 0;
					bzero(temp, 4);
				} else if (i == 7) {
					byte = ntohl(*((uint32_t *)(temp)));
					c = 0;
					bzero(temp, 4);
				} else if (i == 11 && flag == 1) {
					itime = ntohl(*((uint32_t *)(temp)));
					c = 0;
					bzero(temp, 4);
				} else if (i == 15 && flag == 1) {
					dtime = ntohl(*((uint32_t *)(temp)));
					c = 0;
					bzero(temp, 4);
				}
			}
			// construct double
			if (flag == 1) {
				time = (double)itime + ((double)dtime/1000);
			}

			//printf("Flag: %d\nbyte: %d\ntime: %f\n", flag, byte, time);
			
			// Start = 1
			if (flag == 1) {
				insertValue(time, 0, byte);
				printList();
			// Cancel = 2
			} else if (flag == 2) {
				removeValue(byte);
				printList();
			// Quit = 3
			} else if (flag == 3) {
				printf("Recieved Quit Flag. Now Quitting...\n");
				break;
			}
		}
		// unquote if fix select()
		prev = decrement(prev);

	}

	// Close Connections
	close(msgsock);
}