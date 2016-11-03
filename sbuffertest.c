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

void main (int args, char *argv[]) {
	char * data = malloc(64000);
	perror("Initialize");
	initialize();

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
	requestData(data);
	perror("Request Data Print");
	printList();

	// wrap data + merge
	perror("Wrap big data + merge");
	addData(46, 19000, data);
	perror("Wrap big data +merge Print");
	printList();
}