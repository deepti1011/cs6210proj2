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

  if(shm_rbuff == MAP_FAILED) {
    perror("Unable to map shared memory.");
    close(fd);
    shm_unlink(DATA);
    return 1;
  }

  close(fd);
  
  pthread_mutex_lock(&shm_rbuff->request_mutex);
  int x = 10;
  int p = 7;
  pthread_cond_t* response_ready;
  write_request(shm_rbuff, x, p, response_ready);
  pthread_mutex_unlock(&shm_rbuff->request_mutex);

  pthread_mutex_lock(&shm_rbuff->response_mutex);
  pthread_cond_wait(response_ready, &shm_rbuff->response_mutex);
  pthread_mutex_unlock(&shm_rbuff->response_mutex);

  munmap(shm_rbuff, sizeof(struct ring_buffer));
  shm_unlink(DATA);
}
