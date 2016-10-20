
void checkWindow();



int addData(char * data, int size);



int recvACK();



int requestBSN();



int requestSize(int num);


void requestData(int num, char * data);



int isFull();



void initHead();





















/*
//___________________________
//| Andrew Lee				|
//| CSE5462	Project 1		|
//| 10-17-16 cbuffer.h		|
//|_________________________|

// Header file for cbuffer

// Initialize Buffer and Linked List
void initialize();

// Determines buffer status
// 0 if buffer is completely full
// 1 if buffer is not full but cannot fit input size
// 2 if buffer can fit data without issue
// 3 if buffer can fit data but requireds wraparound
int bufStatus(int size);

// Add data to cbuffer
int addData(int size, char *data);

// Remove data from cbuffer
// Returns time difference between SENT and ACK for packet
double freeData(int byteSeqNum);

// Next Packet -- Returns ByteSeqNum for next packet
// -1 if unable to find next packet
int nextPacket();

// Gets size of packet
int getSize(int byteSeqNum);

// Gets data for next packet
// Null if next packet is not part of linked list
char *getData();
*/
