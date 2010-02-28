#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

struct point {
  int x;
  int y;
};

int main() {
  int fd = shm_open("testmem", O_RDWR | O_CREAT, 777);
  struct point pt;
  pt.x = 10;
  pt.y = 21;
  size_t size = sizeof(struct point);
  write(fd, &pt, size);

  struct point * pt2;
  pt2 = (struct point *)mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE, 
       MAP_SHARED, fd, 0);

  if(pt2 != MAP_FAILED) {
    printf("%d %d\n", pt2->x, pt2->y);
  }

  munmap(pt2, size);
  shm_unlink("testmem");
}
