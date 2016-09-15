CC = gcc

OPTIONS = -std=gnu99

ALL:FTPC FTPS TCPDC TCPDS


FTPC:ftpc.c
	$(CC) $(OPTIONS)  srbaclib.c ftpc.c -o ftpc


FTPS:ftps.c
	$(CC) $(OPTIONS)  srbaclib.c ftps.c -o ftps

TCPDC:tcpdc.c
	$(CC) $(OPTIONS)  srbaclib.c tcpdc.c -o tcpdc

TCPDS:tcpds.c
	$(CC) $(OPTIONS)  srbaclib.c tcpds.c -o tcpds

clean:
	rm ftpc ftps tcpdc tcpds
	rm -r recvd
