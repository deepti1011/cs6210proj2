#include <pthread.h>

#define MAXSIZE 128
#define REQUESTS "requests"

struct request {
  int x;
  int p;
};

struct ring_buffer {
  struct request buffer[MAXSIZE];
  struct request* end;
  struct request* start;
  pthread_mutex_t buffer_mutex;
  pthread_cond_t nonempty;
};

void init_ring_buffer(struct ring_buffer* rbuff);
