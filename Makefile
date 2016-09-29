CC = gcc

OPTIONS = -std=gnu99

ALL:FTPC FTPS TCPDC TCPDS TIMR TIMRDEMO


FTPC:ftpc.c tcpapi.c
	$(CC) $(OPTIONS)  tcpapi.c ftpc.c -o ftpc

FTPS:ftps.c tcpapi.c
	$(CC) $(OPTIONS)  tcpapi.c ftps.c -o ftps

TCPDC:tcpdc.c tcpapi.c crc32.c
	$(CC) $(OPTIONS)  tcpapi.c crc32.c tcpdc.c -o tcpdc

TCPDS:tcpds.c tcpapi.c crc32.c
	$(CC) $(OPTIONS)  tcpapi.c crc32.c  tcpds.c -o tcpds

TIMR: timerprocess.c
	$(CC) $(OPTIONS) timerprocess.c -o tp

TIMRDEMO: timerDEMO.c
	$(CC) $(OPTIONS) timerDEMO.c -o td

clean:
	rm ftpc ftps tcpdc tcpds tp td
	rm -r recvd
