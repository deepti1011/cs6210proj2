#include "server.h"
#include <sys/mman.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

void init_ring_buffer(struct ring_buffer* rbuff) {
  rbuff->end = rbuff->buffer;
  rbuff->start = rbuff->buffer;

  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);  
  pthread_mutex_init(&rbuff->buffer_mutex, &mattr);

  pthread_condattr_t  cattr;
  pthread_condattr_init(&cattr);
  pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
  pthread_cond_init(&rbuff->nonempty, &cattr);  
}

int main() {
  struct ring_buffer rbuff;
  init_ring_buffer(&rbuff);
  
  int fd = shm_open(REQUESTS, O_RDWR | O_CREAT, S_IRWXU | S_IRWXO);

  write(fd, &rbuff, sizeof(struct ring_buffer));
  struct ring_buffer* shm_rbuff;

  shm_rbuff = (struct ring_buffer *)mmap(0, sizeof(struct ring_buffer),
					 PROT_EXEC | PROT_READ | PROT_WRITE,
					 MAP_SHARED, fd, 0);

  if(shm_rbuff == MAP_FAILED) {
    perror("Unable to map shared memory.");
    close(fd);
    shm_unlink(REQUESTS);
    return 1;
  }

  close(fd);
  
  while(1) {
    pthread_mutex_lock(&shm_rbuff->buffer_mutex);
    pthread_cond_wait(&shm_rbuff->nonempty, &shm_rbuff->buffer_mutex);
  }  

  munmap(shm_rbuff, sizeof(struct ring_buffer));
  shm_unlink(REQUESTS);
}
