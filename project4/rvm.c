#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "rvm.h"

#define MAXSEGS 30

segment mapped[MAXSEGS];
int num_mapped;
int verbose = 0;
long tid = 0;

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
  close(open(buffer, O_RDONLY | O_CREAT, S_IRWXU));

  char logMessage[100];
  strcpy(logMessage, "RVM_INIT ");
  strcat(logMessage, "\n");
  logRVM(store, logMessage);

  return store;
}

void* rvm_map(rvm_t rvm, const char *segname, int size_to_create) {
  int dirlen, seglen, fd, remains, i, log;
  void * result;

  seglen = strlen(segname);
  
  if(num_mapped == MAXSEGS) {
    fprintf(stderr, "Maximum number of segments reached.\n");
    return NULL;
  }
  
  char buffer[1024];
  strcpy(buffer, rvm.dir);
  strcat(buffer, segname);

  if((fd = open(buffer, O_RDWR | O_CREAT, S_IRWXU)) == -1) {
    perror("Unable to open segment file");
    return NULL;
  }

  /*strcat(buffer, ".log");
  log = open(buffer, O_RDWR | O_CREAT, S_IRWXU);
  printf("Locking log for segment.\n");
  write_lock(log);
  printf("Lock successful.\n");*/
  
  lockf(fd, F_LOCK, size_to_create);

  struct stat info;
  if(fstat(fd, &info) != 0) {
    perror("Unable to get file info");
    close(fd);
    return NULL;
  }

  
  
  i = 0;
  for(i = 0; i < num_mapped; i++) {
    if(strcmp(mapped[i].name, segname) == 0) {
      fprintf(stderr, "Illegal attempt to map segment\n");
      close(fd);
      return NULL;
    } 
  }

  remains = size_to_create - info.st_size;
  char padding[remains];
  memset(padding, '\0', remains);

  i = 0;
  lseek(fd, 0, SEEK_END);
  while((i += write(fd, padding, remains - i)) < remains);

  result = mmap(NULL, size_to_create, 
		PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  
  if(remains == size_to_create) {
    mapped[num_mapped].data = result;
    mapped[num_mapped].size = size_to_create;    
    mapped[num_mapped].name = (char*)malloc(seglen * sizeof(char));
    mapped[num_mapped].ranges = (struct range*) malloc(10 * sizeof(struct range));
    strcpy(mapped[num_mapped].name, segname); 
    num_mapped++;
  } else {
    for(i = 0; i <= num_mapped; i++) {
      if(mapped[i].data == result) {
	mapped[i].size = size_to_create;
	break;
      } else if(i == num_mapped) {
	mapped[num_mapped].data = result;
	mapped[num_mapped].size = size_to_create;
	mapped[num_mapped].name = (char*)malloc(seglen * sizeof(char));
        mapped[num_mapped].ranges = (struct range*) malloc(10 * sizeof(struct range));
	strcpy(mapped[num_mapped].name, segname);
	num_mapped++;
	break;
      }
    }
  }

  i = 0;
  lseek(fd, -1 * size_to_create, SEEK_END);
  while((i += write(fd, result + i, size_to_create - i)) < size_to_create);
   
  char logMessage[100];
  strcpy(logMessage, "RVM_MAP SEGMENT ");
  strcat(logMessage, mapped[num_mapped -1].name);
  strcat(logMessage, "\n");
  logRVM(rvm, logMessage);
  logSegment(rvm, mapped[num_mapped - 1], logMessage);
  // lockf(fd, F_ULOCK, size_to_create); //Closing the fd has the same effect
  close(fd);

  /*unlock(log);
  printf("Segment file unlocked.\n");
  close(log);*/
  memset(buffer, '\0', 1024);
  strcpy(buffer, rvm.dir);
  strcat(buffer, "segments.log");
  fd = open(buffer, O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
 
  memset(buffer, '\0', 1024);
  strcpy(buffer, segname);
  buffer[seglen] = '\n';
  write(fd, buffer, seglen + 1);
  close(fd);

  return result;
}

/*
void write_lock(int fd) {
  lockf(fd, F_LOCK, 
}

void unlock(int fd) {
 
}*/

void rvm_unmap(rvm_t rvm, void *segbase) {
  int i;
  for(i = 0; i < num_mapped; i++) {
    
    if(segbase == mapped[i].data) {
      //printf("Found segment to unmap.\n");
      if(munmap(segbase, mapped[i].size) != 0) {
	perror("Unable to unmap memory.");
	return;
      } else {
        char logMessage[100];
  	strcpy(logMessage, "RVM_UNMAP SEGMENT ");
  	strcat(logMessage, mapped[i].name);
  	strcat(logMessage, "\n");
  	logRVM(rvm, logMessage);
  	logSegment(rvm, mapped[i], logMessage);
	break; 
      }

    } else if(i == num_mapped - 1) {
      fprintf(stderr, "No record of memory to unmap.\n");
      return;
    }
  }
  for(;i < num_mapped - 1; i++) {
    mapped[i] = mapped[i + 1];
  }
  num_mapped--;
}

void rvm_destroy(rvm_t rvm, const char *segname) {
  char buffer[1024];

  strcpy(buffer, rvm.dir);
  strcat(buffer, segname);
  remove(buffer);

  strcat(buffer, ".log");
  remove(buffer);
  
  char logMessage[100];
  strcpy(logMessage, "RVM_DESTROY SEGMENT ");
  strcat(logMessage, segname);
  strcat(logMessage, "\n");
  logRVM(rvm, logMessage);
}

void lock_segment(rvm_t rvm , segment seg) {
  int dirlen, seglen, fd;
  dirlen = strlen(rvm.dir);
  seglen = strlen(seg.name);

  char buffer[dirlen + seglen + 5];
  strcpy(buffer, rvm.dir);
  strcat(buffer, seg.name);
  strcat(buffer, ".log");

  fd = open(buffer, O_RDWR, S_IRWXU);
  lockf(fd, F_LOCK, seg.size);
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases) {
  int i, j;
  trans_t trans;
  trans.rvm = rvm;
  trans.tid = tid++; /*not threadsafe*/
  trans.numsegs = numsegs;
  trans.segbases = segbases;

  for(j = 0; j < numsegs; j++) {
    for(i = 0; i < num_mapped; i++) {
      if(mapped[i].data == segbases[j]) {
	lock_segment(rvm, mapped[i]);
	char logMessage[100];
  	strcpy(logMessage, "RVM_BEGINTRANS SEGMENT ");
  	strcat(logMessage, mapped[i].name);
  	strcat(logMessage, "\n");
	logRVM(rvm, logMessage);
 	logSegment(rvm, mapped[i], logMessage);
      }
    }
  }
  return trans;
}

void write_segment(rvm_t rvm, segment seg) {
  int dirlen, seglen, fd, i, itr;
  dirlen = strlen(rvm.dir);
  seglen = strlen(seg.name);

  char buffer[dirlen + seglen + 5];
  strcpy(buffer, rvm.dir);
  strcat(buffer, seg.name);
  fd = open(buffer, O_RDWR, S_IRWXU);
  //printf("Writing Segment with %d ranges\n", seg.numRanges);
  for(itr = 0; itr < seg.numRanges; itr++)
  {
	int offset = seg.ranges[itr].offset;
        int size = seg.ranges[itr].size;
	//printf("Writing to range (%d, %d)\n", offset, size);
  	lseek(fd, -1 * seg.size + offset, SEEK_END);
  	i = 0;
  	while((i += write(fd, seg.data + offset, size)) < size);
  }
  //lockf(fd, F_ULOCK, seg.size); //Unnecessary, close has the same effect
  close(fd);
}

void rvm_commit_trans(trans_t trans) {
  int i, j, fd;
   for(i = 0; i < trans.numsegs; i++) {
     for(j = 0; j < num_mapped; j++) {
       if(trans.segbases[i] == mapped[j].data) {
	 write_segment(trans.rvm, mapped[j]);
	 char logMessage[100];
  	 strcpy(logMessage, "RVM_COMMITTRANS SEGMENT ");
  	 strcat(logMessage, mapped[j].name);
  	 strcat(logMessage, "\n");
	 logRVM(trans.rvm, logMessage);
 	 logSegment(trans.rvm, mapped[j], logMessage);
       }
     }
   }
}

segment* getSegmentFromSegBase(void* segbase)
{	
  	int i;
  	for(i=0; i < num_mapped; i++)
  	{
		if(segbase == mapped[i].data)
		{
			return &mapped[i];
		}
  	}
	return NULL;
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size)
{  	
	segment* seg;
	struct range rng;

	rng.offset = offset;
	rng.size = size;
    	seg = getSegmentFromSegBase(segbase);
    	if(seg != NULL)
    	{
		seg->ranges[seg->numRanges] = rng;
		seg->numRanges++;
		char logMessage[250];
  		sprintf(logMessage, "RVM_MODIFY SEGMENT %s OFFSET: %d SIZE: %d", seg->name, offset, size);
  		strcat(logMessage, "\n");
		logRVM(tid.rvm, logMessage);
 		logSegment(tid.rvm, *seg, logMessage);
	}
}

void rvm_abort_trans(trans_t tid)
{
  int i, j, k, size;
  
   for(i = 0; i < tid.numsegs; i++) {
     //printf("Aborting segment %d\n", i);
     for(j = 0; j < num_mapped; j++) {
       if(tid.segbases[i] == mapped[j].data) {
	int dirlen, seglen, fd;
  	dirlen = strlen(tid.rvm.dir);
  	seglen = strlen(mapped[j].name);

  	char buffer[dirlen + seglen + 5];
  	strcpy(buffer, tid.rvm.dir);
  	strcat(buffer, mapped[j].name);
  	fd = open(buffer, O_RDWR, S_IRWXU);
  	char logMessage[100];
  	strcpy(logMessage, "RVM_ABORTTRANS ");
  	strcat(logMessage, mapped[j].name);
  	strcat(logMessage, "\n");
 	logSegment(tid.rvm, mapped[j], logMessage);
	
	k = 0;
	//printf("Get segment size in abort.\n");
	size = mapped[j].size;
	while(k < size) {
	  if(mapped[j].data == NULL)
	    printf("Your segment data pointer is null.\n");

	  k += read(fd, mapped[j].data + k, size - k);
	}
	char logMessageRVM[100];
  	strcpy(logMessageRVM, "RVM_ABORTTRANS ");
  	strcat(logMessageRVM, "\n");
	logRVM(tid.rvm, logMessageRVM);
			       
	//Unnecessary, close has the same effect
	//lockf(fd, F_ULOCK, mapped[j].size); 
  	close(fd);
	}
     }
   }	
}

void logRVM(rvm_t rvm, char* logMessage)
{
  int dirlen, seglen, msglen, fd, i;
  dirlen = strlen(rvm.dir);
  msglen = strlen(logMessage);

  char buffer[dirlen + 10];
  strcpy(buffer, rvm.dir);
  strcat(buffer, "store.log");
  fd = open(buffer, O_RDWR | O_APPEND, S_IRWXU);
  i = 0;
  while((i += write(fd, logMessage + i, msglen - i)) < msglen);
  
  if(verbose != 0)
  {
  	printf("%s", logMessage);
  }
  //lockf(fd, F_ULOCK, seg.size); //Unnecessary, close has the same effect
  close(fd);
}

void logSegment(rvm_t rvm, segment seg, char* logMessage)
{
  int dirlen, seglen, msglen, fd, i;
  dirlen = strlen(rvm.dir);
  seglen = strlen(seg.name);
  msglen = strlen(logMessage);
  char buffer[dirlen + seglen + 5];
  strcpy(buffer, rvm.dir);
  strcat(buffer, seg.name);
  strcat(buffer, ".log");
  fd = open(buffer, O_RDWR | O_APPEND | O_CREAT, S_IRWXU);
  i = 0;
  while((i += write(fd, logMessage + i, msglen - i)) < msglen);
  
  //lockf(fd, F_ULOCK, seg.size); //Unnecessary, close has the same effect
  close(fd);
}

void rvm_verbose(int enable_flag)
{
	verbose = enable_flag;
}

void rvm_truncate_log(rvm_t rvm)
{
  int i;
  FILE* file;
  char buffer[1024], segment[1024];
  strcpy(buffer, rvm.dir);
  strcat(buffer, "segments.log");
  file = fopen(buffer, "r+");

  memset(buffer, '\0', 1024);
  fgets(buffer, 1024, file);

  while(buffer[0] != '\0') { 
    for(i = 0; buffer[i] != '\n' && buffer[i] != '\0'; i++);
    buffer[i] = '\0';

    memset(segment, '\0', 1024);
    strcpy(segment, rvm.dir);
    strcat(segment, buffer);
    strcat(segment, ".log");
    
    open(segment, O_RDWR | O_TRUNC, S_IRWXU);
    memset(buffer, '\0', 1024);
    fgets(buffer, 1024, file);   
  }
  
  //truncate main log with playthrough
  int fd, j;
  char log[1024];
  
  memset(log, '\0', 1024);
  strcpy(log, rvm.dir);
  strcat(log, "store.log");
  fd = open(log, O_RDONLY, S_IRWXU);
  
  j = 0;
  memset(buffer, '\0', 1024);
  while ((j = read(fd, buffer, 1023)) > 0) {
  buffer[j] = '\0';
  printf("%s", buffer);
  }

  close(fd);
  open(log, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
}
/*
int main() {
  rvm_t store;
  void* data;
  int size;
  
  store = rvm_init("store");
  rvm_destroy(store, "two");
  data = rvm_map(store, "two", 1024);
  /*rvm_unmap(store, data);
    rvm_destroy(store, "one");*/

/*free(store.dir);
}*/
