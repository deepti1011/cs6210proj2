#include "ringbuffer.h"
#include <sys/mman.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
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
  
  int x = 10;
  int p = 7;
    
  int response = write_request(shm_rbuff, x, p);
  
  pthread_mutex_lock(&shm_rbuff->data_mutex);
  pthread_cond_wait(&shm_rbuff->response_ready[response], &shm_rbuff->data_mutex);
  
  int result;
  result = shm_rbuff->response_buffer[shm_rbuff->response_reads % MAXSIZE];
  printf("Request (%d, %d) = %d\n", x, p, result);
  shm_rbuff->response_reads++;
  pthread_mutex_unlock(&shm_rbuff->data_mutex);

  munmap(shm_rbuff, sizeof(struct ring_buffer));
  shm_unlink(DATA);
}
