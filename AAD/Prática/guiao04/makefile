#
# makefile for the practical class AAD_P04
#

clean:
	rm -f a.out client server

server:		server_clients.c
	cc -Wall -O2 -DSERVER=1 server_clients.c -o server

client:		server_clients.c
	cc -Wall -O2 -DSERVER=0 server_clients.c -o client

server_parallelization:		parallelization.c
	cc -Wall -O2 -DSERVER=1 parallelization.c -o server_parallelization

client_parallelization:		parallelization.c
	cc -Wall -O2 -DSERVER=0 parallelization.c -o client_parallelization

all: server client server_parallelization client_parallelization
