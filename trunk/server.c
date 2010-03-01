#include "ringbuffer.h"
#include <sys/mman.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  struct ring_buffer rbuff;
  init_ring_buffer(&rbuff);
  
  int fd = shm_open(DATA, O_RDWR | O_CREAT, S_IRWXU | S_IRWXO);

  write(fd, &rbuff, sizeof(struct ring_buffer));
  struct ring_buffer* shm_rbuff;

  shm_rbuff = (struct ring_buffer *)mmap(0, sizeof(struct ring_buffer),
					 PROT_EXEC | PROT_READ | PROT_WRITE,
					 MAP_SHARED, fd, 0);

  if(shm_rbuff == MAP_FAILED) {
    perror("Unable to map shared memory.");
    close(fd);
    shm_unlink(DATA);
    return 1;
  }

  close(fd);
  
  while(1) {
    pthread_mutex_lock(&shm_rbuff->request_mutex);
    pthread_cond_wait(&shm_rbuff->nonempty, &shm_rbuff->request_mutex);
    printf("new requests\n");
    pthread_mutex_lock(&shm_rbuff->response_mutex);
    printf("response lock acquired\n");
    process_requests(shm_rbuff);
    printf("requests processed\n");
    pthread_mutex_unlock(&shm_rbuff->response_mutex);
    pthread_mutex_unlock(&shm_rbuff->request_mutex);
  }  

  munmap(shm_rbuff, sizeof(struct ring_buffer));
  shm_unlink(DATA);
}
