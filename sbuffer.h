//___________________________
//| Andrew Lee				|
//| CSE5462	Project 1		|
//| 10-17-16 sbuffer.h		|
//|_________________________|

// Header file for sbuffer

// Initialize
void initialize();

// Inserts value at appropriate position
int addData(int byteSeqNum, int size, char *data);

// gets size of first node (if it contains data)
int getSize();

// Checks and gets data
int requestData(int size, char * temp);

// free data
//void freeBuffer(char *buffer);

// Prints the current Linked List
int printList();

// merge list (troubleshooting scope)
//void mergeList();
