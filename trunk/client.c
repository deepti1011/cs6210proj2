#include "ringbuffer.h"
#include <sys/mman.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <sys/time.h>

#define PRIME 97
#define INTERVAL 10
#define NUM_THREADS 128

void* make_requests(void* param) {
  struct ring_buffer* rbuff = (struct ring_buffer*)param; 

  int i;
 loop:
  for  (i = 0; pow(2.0, i) < INT_MAX; i++) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int response = write_request(rbuff, i, PRIME);

    if(response == -1)
      continue;

    pthread_mutex_lock(&rbuff->data_mutex);
    pthread_cond_wait(&rbuff->response_ready[response],
		      &rbuff->data_mutex);

    int result;
    result = rbuff->response_buffer[rbuff->response_reads % MAXSIZE];
    
    rbuff->response_reads++;
    pthread_mutex_unlock(&rbuff->data_mutex);
    gettimeofday(&end, NULL);
    unsigned int time = end.tv_usec - start.tv_usec;
    printf("Request (%d, %d) = %d. Time elapsed(us) %u\n", i, PRIME, 
	   result, time);

    usleep(10);
  }
}

int main() {
  pthread_t threads[NUM_THREADS];
  int fd = shm_open(DATA, O_RDWR | O_CREAT, S_IRWXU | S_IRWXO);

  struct ring_buffer* shm_rbuff;
  shm_rbuff = (struct ring_buffer*)mmap(0, sizeof(struct ring_buffer),
					PROT_EXEC | PROT_READ | PROT_WRITE,
					MAP_SHARED, fd, 0);
  close(fd);
  if(shm_rbuff == MAP_FAILED) {
    perror("Unable to map shared memory.");
    shm_unlink(DATA);
    return 1;
  }
  
  int i;
  for(i = 0; i < NUM_THREADS; i++) {
    if((pthread_create(&threads[i], NULL, make_requests, 
		       (void*)shm_rbuff)) != 0) {
      perror("Unable to create thread");
      break;
    }
  }

  for(i = 0; i < NUM_THREADS; i++) 
    pthread_join(threads[i], NULL);

  munmap(shm_rbuff, sizeof(struct ring_buffer));
  shm_unlink(DATA);
}
