#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

int main() {
  int fd = shm_open("testmem", O_RDWR | O_CREAT, 777);
  char request[] = "test";
  size_t size = sizeof(request);
  write(fd, request, size);

  void* received;
  received = (char*)mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE, 
       MAP_SHARED, fd, 0);

  if(received != MAP_FAILED) {
    bzero(request, size);
    snprintf(request, size, "%s", (char*)received);
    printf("%s\n", request);
  }

  munmap(received, size);
  shm_unlink("testmem");
}
