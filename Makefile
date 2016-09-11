CC = gcc

OPTIONS = -std=gnu99

ALL:FTPC FTPS


FTPC:ftpc.c
	$(CC) $(OPTIONS) ftpc.c -o ftpc


FTPS:ftps.c
	$(CC) $(OPTIONS) ftps.c -o ftps


clean:
	rm ftpc ftps
	rm -r recvd