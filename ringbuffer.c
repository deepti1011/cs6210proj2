#include "ringbuffer.h"
#include <math.h>
#include <sys/mman.h>
#include <stdio.h>

void read_request(struct ring_buffer* rbuff) {
  pthread_mutex_lock(&rbuff->data_mutex);
  int size = rbuff->request_writes - rbuff->request_reads;

  if(size < 1)
    return;

  struct request* next = &rbuff->request_buffer[rbuff->request_reads % MAXSIZE];
  int result = pow(2.0, next->x);
  result = result % next->p;
  /*printf("Received request (%d, %d) = %d\n", next->x, next->p, result);*/
  rbuff->request_reads++;
  
  rbuff->response_buffer[rbuff->response_writes % MAXSIZE] = result;
  rbuff->response_writes++;
  
  pthread_cond_signal(&rbuff->response_ready[next->response]);
  pthread_mutex_unlock(&rbuff->data_mutex);
}

void process_requests(struct ring_buffer* rbuff) {
  while((rbuff->request_writes - rbuff->request_reads) > 0) 
    read_request(rbuff);
}

int write_request(struct ring_buffer* rbuff, int x, int p) {
  struct request* next;

  pthread_mutex_lock(&rbuff->data_mutex);
  int size = rbuff->request_writes - rbuff->request_reads;

  if(size > MAXSIZE - 1)
    return -1;

  next = &rbuff->request_buffer[rbuff->request_writes % MAXSIZE];
  
  next->x = x;
  next->p = p;
  rbuff->request_writes++;
  int response = next->response;

  pthread_cond_signal(&rbuff->nonempty);
  pthread_mutex_unlock(&rbuff->data_mutex);
  return response;
}

void init_ring_buffer(struct ring_buffer* rbuff) {
  rbuff->request_writes = 0;
  rbuff->request_reads = 0;

  rbuff->response_writes = 0;
  rbuff->response_reads = 0;

  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);  
  pthread_mutex_init(&rbuff->data_mutex, &mattr);
  
  pthread_condattr_t  cattr;
  pthread_condattr_init(&cattr);
  pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
  pthread_cond_init(&rbuff->nonempty, &cattr);

  int i;
  for(i = 0; i < MAXSIZE; i++) {
    pthread_cond_init(&rbuff->response_ready[i], &cattr);
    rbuff->request_buffer[i].response = i;
  }
}
