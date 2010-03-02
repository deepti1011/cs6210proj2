all: 
	gcc -lm -lpthread -lrt -o server server.c ringbuffer.c
	gcc -lm -lpthread -lrt -o client client.c ringbuffer.c
	./server & ./client

