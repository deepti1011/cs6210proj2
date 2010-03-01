#include <pthread.h>

#define MAXSIZE 128
#define DATA "data"

struct request {
  int x;
  int p;
  pthread_cond_t response_ready;
};

struct ring_buffer {
  struct request request_buffer[MAXSIZE];
  unsigned int request_writes;
  unsigned int request_reads;
  pthread_mutex_t request_mutex;
  pthread_cond_t nonempty;

  int response_buffer[MAXSIZE];
  unsigned int response_writes;
  unsigned int response_reads;
  pthread_mutex_t response_mutex;
};

void init_ring_buffer(struct ring_buffer* rbuff);
void write_request(struct ring_buffer* rbuff, int x, int p, 
		   pthread_cond_t* response_ready);
void read_request(struct ring_buffer* rbuff);
void process_requests(struct ring_buffer* rbuff);

