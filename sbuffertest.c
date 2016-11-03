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

char * data;
 
void standardTest() {
	// Initial Print
	perror("Empty Print");
	printList();

	// simple add test
	perror("Added");
	addData(1, 1000, data);
	perror("NonEmpty Print");
	printList();

	// add with hole
	perror("Added hole");
	addData(3, 1000, data);
	perror("Hole Print");
	printList();

	// add big hole
	perror("Added big hole");
	addData(45, 1000, data);
	perror("Big Hole Print");
	printList();

	// fill hole
	perror("Fill hole");
	addData(2, 1000, data);
	perror("Fill Hole Print");
	printList();

	// remove first packet
	perror("Request Data");
	printf("got %d\n",requestData(100000, data));
	perror("Request Data Print");
	printList();

	// wrap data + merge
	perror("Wrap big data + merge");
	addData(46, 1000, data);
	perror("Wrap big data +merge Print");
	printList();

	// Continue writing
	perror("Continued");
	addData(54, 1000, data);
	perror("Continued Print");
	printList();

	// End merge
	perror("Fill end hole");
	addData(53, 1000, data);
	perror("Fill end Hole Print");
	printList();

	// repeat
	perror("Repeat");
	addData(53, 1000, data);
	perror("Repeat Print");
	printList();

	// Insert Past value
	perror("too large seqnum");
	addData(69, 1000, data);
	perror("large print");
	printList();

	// Insert 4,5,6
	perror("adding more data");
	addData(4, 1000, data);
	addData(5, 1000, data);
	addData(6, 1000, data);
	printList();

	printf("got %d\n",requestData(100000, data));
	printList();
}

void fillTest() {
	// Fill buffer
	for (int i = 1; i < 65; i++) {
		addData(i, 1000, data);
	}
	printList();
	printf("got %d\n",requestData(100000, data));
	printList();
	printf("got %d\n",requestData(100000, data));
	printList();
}

void main (int args, char *argv[]) {
	data = malloc(64000);
	perror("Initialize");
	initialize();

	standardTest();
	//fillTest();
}