#################################################################
##
## FILE:    Makefile
## PROJECT: CS 3251 Project 1 - Professor Ellen Zegura 
## DESCRIPTION: Compile Project 1
## CREDIT:  Adapted from Professor Traynor
##
#################################################################

CC=gcc

OS := $(shell uname -s)

# Extra LDFLAGS if Solaris
ifeq ($(OS), SunOS)
    LDFLAGS=-lsocket -lnsl
    endif

all: client server client2 server2

client: client.c
	$(CC) client.c -o client

server: server.c
	$(CC) server.c -o server

client2: client2.c
	$(CC) client2.c -o client2

server2: server2.c
	$(CC) server2.c -o server2

clean:
	rm -f client server client2 server2 *.o

