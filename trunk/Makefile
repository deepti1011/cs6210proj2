all: server client

server:	server.o
	gcc -o server server.o

client: client.o
	gcc -o client client.o

server.o: server.c ringbuffer.c
	gcc -lm -lpthread -lrt -o server.o server.c ringbuffer.c

client.o: client.c ringbuffer.c
	gcc -lm -lpthread -lrt -o client.o client.c ringbuffer.c
