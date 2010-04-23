#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "rvm.h"

#define MAXSEGS 8

segment mapped[MAXSEGS];
int num_mapped;

rvm_t rvm_init(const char *directory) {
  int dirlen;
  rvm_t store;
  char buffer[1024];

  num_mapped = 0;
  
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

  if(num_mapped == MAXSEGS) {
    fprintf(stderr, "Maximum number of segments reached.\n");
    return NULL;
  }

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

  i = 0;
  while((i += write(fd, padding, remains - i)) < remains);

  result = mmap(NULL, size_to_create, 
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  
  if(remains == size_to_create) {
    mapped[num_mapped].data = result;
    mapped[num_mapped].size = size_to_create;
    num_mapped++;
  } else {
    for(i = 0; i < num_mapped; i++) {
      if(mapped[i].data == result) {
	mapped[i].size = size_to_create;
	break;
      } else if(i == num_mapped - 1) {
	mapped[num_mapped].data = result;
	mapped[num_mapped].size = size_to_create;
	num_mapped++;
	break;
      }
    }
  }

  close(fd);
  return result;
}

void rvm_unmap(rvm_t rvm, void *segbase) {
  int i;
  for(i = 0; i < num_mapped; i++) {
    if(segbase == mapped[i].data) {
      
      if(munmap(segbase, mapped[i].size) != 0) {
	perror("Unable to unmap memory.");
	return;
      } else { break; }
    } else if(i == num_mapped - 1) {
      fprintf(stderr, "No record of memory to unmap.\n");
      return;
    }
  }

  for(;i < num_mapped - 1; i++) {
    mapped[i].data = mapped[i + 1].data;
    mapped[i].size = mapped[i + 1].size;
  }
  num_mapped--;
}

void rvm_destroy(rvm_t rvm, const char *segname) {
  int name_len, dir_len;
  
  name_len = strlen(segname);
  dir_len = strlen(rvm.dir);

  char buffer[dir_len + name_len + 1];

  strcpy(buffer, rvm.dir);
  strcat(buffer, segname);
  
  remove(buffer);
}

int main() {
  rvm_t store;
  void* data;
  int size;
  
  store = rvm_init("store");
  data = rvm_map(store, "one", 1024);
  rvm_unmap(store, data);
  rvm_destroy(store, "one");

  free(store.dir);
}
