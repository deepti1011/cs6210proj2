#include <fcntl.h>
#include <stdio.h>
#include <string.h>

struct request { 
  int x;
  int p;
  char name[10];
  } a;

int main() 
{
  FILE* file = fopen("testfile", "wb");

  printf("file opened\n");
  a.x = 10;
  a.p = 11;
  strcpy(a.name, "bob");
  
  printf("prewrite\n");
  int status = fwrite(&a, sizeof(struct request), 1, file);
  printf("status1: %d\n", status);

  fclose(file);

  file = fopen("testfile", "rb");

  struct request b;

  status = fread(&b, sizeof(struct request), 1, file);
  printf("status2: %d\b", status);

  printf("%d %d\n", b.x, b.p);
  fclose(file);
}
