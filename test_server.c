#include <stdio.h>
#include <stdlib.h>
#include <fildes.h>
#include "test_server.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

void readRequestFile()
{
	FILE *fd;
	size_t nio;
	int fildes = shm_open(fname, O_RDONLY | O_CREAT, 777);
	
	if (!(fd = fdopen(fildes, "rb"))) {
		printf("Could not open \"%s\" for output, exiting...", fname);
		exit(EXIT_FAILURE);
	}
	//TODO: lock the file

	//read the request file
	Request[RING_BUFFER_SIZE] tempRingBuffer;
	printf("Attempting to read the structure.\n");
	int nio = fread(&tempRingBuffer, sizeof tempRingBuffer, RING_BUFFER_SIZE, fd);
	//TODO: unlock the file
	
	printf("%lu units read.\n", nio);
	
	fclose(fd);
	
	//TODO: check new requests and update ring buffer
	
	//TODO: if file looks mucked by client, rewrite ring buffer
}

void writeRequestFile()
{	
	FILE *fd;
	size_t nio;
	int fildes = shm_open(fname, O_RDWR | O_CREAT, 777);
	
	if (!(fd = fopen(fildes, "wb"))) {
		printf("Could not open \"%s\" for output, exiting...", fname);
		exit(EXIT_FAILURE);
	}
	//TODO: lock the file

	//write the request file
	printf("Attempting to read the structure.\n");
	int nio = fwrite(&ringBuffer, sizeof ringBuffer, RING_BUFFER_SIZE, fd);
	//TODO: unlock the file
	
	printf("%lu units written.\n", nio);
	
	fclose(fd);
}

void checkFileForUpdates()
{
  
}

int main(int argc, char **argv)
{
	//Initialize Ring Buffer
	int i;
	for(i = 0; i < RING_BUFFER_SIZE; i++)
	{
		ringBuffer[i].requestID = i
		ringBuffer[i].status = (int)FREE;
	}
	
	//loop through listening for new requests
	while
	{
		checkFileForUpdates();
	}
	return 0;
}