#include <pthread.h>

#define MAXSIZE 128
#define DATA "data"

struct request {
  int x;
  int p;
  struct response* result;
};

struct response {
  int result;
  pthread_cond_t response_ready;
};

struct ring_buffer {
  struct request request_buffer[MAXSIZE];
  int request_end;
  int request_start;
  pthread_mutex_t request_mutex;
  pthread_cond_t nonempty;

  struct response response_buffer[MAXSIZE];
  int response_end;
  int response_start;
  pthread_mutex_t response_mutex;
};

void init_ring_buffer(struct ring_buffer* rbuff);

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
