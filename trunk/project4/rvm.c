#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "rvm.h"

rvm_t rvm_init(const char *directory) {
  int dirlen;
  rvm_t store;
  char buffer[1024];

  mkdir(directory, S_IRWXU);

  dirlen = strlen(directory);

  store.dir = (char*)malloc((dirlen + 2) * sizeof(char));
  
  strcpy(store.dir, directory);

  if(store.dir[dirlen - 1] != '/') {
    store.dir[dirlen] = '/';
    store.dir[dirlen + 1] = '\0';
  }

  strcpy(buffer, store.dir); 
  strcat(buffer, "store.log");

  /*Create log file if it does not already exist*/
  close(open(buffer, O_RDONLY | O_CREAT | O_TRUNC, S_IRWXU));

  return store;
}

void* rvm_map(rvm_t rvm, const char *segname, int size_to_create) {
  int dirlen, seglen, fd, remains, i;
  void * result;

  dirlen = strlen(rvm.dir);
  seglen = strlen(segname);

  char buffer[dirlen + seglen + 1];
  strcpy(buffer, rvm.dir);
  strcat(buffer, segname);

  if((fd = open(buffer, O_RDWR | O_APPEND | O_CREAT, S_IRWXU)) == -1) {
    perror("Unable to open segment file");
    return NULL;
  }

  struct stat info;
  if(fstat(fd, &info) != 0) {
    perror("Unable to get file info");
    return NULL;
  }
  
  if(size_to_create <= info.st_size) {
    fprintf(stderr, "Illegal attempt to map segment\n");
    return NULL;
  }

  remains = size_to_create - info.st_size;
  char padding[remains];
  memset(padding, '\0', remains);

  while((i += write(fd, padding, remains - i)) < remains);

  result = mmap(NULL, size_to_create, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);

  return result;
}

int main() {
  rvm_t store;
  void* data;
  
  store = rvm_init("store");
  rvm_map(store, "one", 1024);

  free(store.dir);
}
