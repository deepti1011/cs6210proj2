#include <stdio.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h> 
#include <string.h>

#define NUM_THREADS 128
#define INTERVAL 10 //in milliseconds

pthread_t threads[NUM_THREADS];
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;


/*
Make Request Should Write to the Request file
and return the requestID, make request should
also lock the file and trigger the mutex cond
variable so the server can operate on it.
*/
int make_request()
{
	return 0;	
}
/*
Check Response should return 0 if response has
not been received and 1 if it has, note this response
takes in a requestID to be matched with the response.
*/
int check_response(int requestID)
{
	return 0;
}
/*
This is just a helper to extract the return value 
from the service.
*/
int get_response_value(int requestID)
{
	return 0;
}

void *StartThread(void *threadid)
{
   int received_response, requestID, retVal;
   long tid;
   tid = (long)threadid;
   pthread_mutex_lock( &mutex1 );
   requestID = make_request();
   printf("Thread %ld has made a request and is awaiting response...\n", tid);
   received_response = 0;
   while(received_response == 0)
   {
	received_response = check_response(requestID);
	if(received_response == 1)
	{
		retVal = get_response_value(requestID);
	}
   }
   
   pthread_mutex_unlock( &mutex1 );
   printf("Thread %ld Finished with response: %d\n", tid, retVal);
}

int main (int argc, char *argv[])
{
   int rc;
   int inx;
   long t;
   for(t=0; t<NUM_THREADS; t++){
      rc = pthread_create(&threads[t], NULL, StartThread, (void *)t);
      if (rc){
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }
      usleep(10);
   }
   for(inx=0; inx<NUM_THREADS; inx++)
		pthread_join(threads[inx], NULL);
}

