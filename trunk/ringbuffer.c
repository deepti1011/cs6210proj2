#include "ringbuffer.h"

void write_request(struct ring_buffer* rbuff, int x, int p, 
		   pthread_cond_t* response_ready) {
  rbuff->request_buffer[rbuff->request_end].x = x;
  rbuff->request_buffer[rbuff->request_end].p = p;
  rbuff->request_buffer[rbuff->request_end].result = 
    &rbuff->response_buffer[rbuff->response_end];
  rbuff->request_end++;
  
  response_ready = &rbuff->response_buffer[rbuff->response_end].response_ready;
  rbuff->response_end++;
  pthread_cond_signal(&rbuff->nonempty);
  
  if(rbuff->response_end == MAXSIZE) {
    rbuff->response_end = 0;
  }
  
  if(rbuff->request_end == MAXSIZE) {
    rbuff->request_end = 0;
  }
}

void init_ring_buffer(struct ring_buffer* rbuff) {
  rbuff->request_end = 0;
  rbuff->request_start = 0;

  rbuff->response_end = 0;
  rbuff->response_start = 0;

  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);  
  pthread_mutex_init(&rbuff->request_mutex, &mattr);
  pthread_mutex_init(&rbuff->response_mutex, &mattr);

  pthread_condattr_t  cattr;
  pthread_condattr_init(&cattr);
  pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
  pthread_cond_init(&rbuff->nonempty, &cattr);

  int i;
  for(i = 0; i < MAXSIZE; i++) {
    pthread_cond_init(&rbuff->response_buffer[i].response_ready, &cattr);
  }
}
