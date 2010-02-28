#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#define MAXSIZE 128

struct request {
  int x;
  int p;
};

struct request* start;
struct request* end;

int main() {
  int fd = shm_open("requests", O_RDWR, S_IRWXU | S_IRWXO);
  
  perror("shm_open"); 
  start = end = (struct request *)mmap(0, sizeof(struct request) * MAXSIZE,
				       PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(start == MAP_FAILED) {
    perror("Unable to map shared memory.");
    close(fd);
    shm_unlink("requests");
    return 1;
  }

  close(fd);
  printf("%d %d\n",start->x, start->p);
  
  shm_unlink("requests");
}
