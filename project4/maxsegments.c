/* basic.c - test that basic persistency works */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"
#define OFFSET2 1000
#define NUM_SEGS 31

/* proc1 writes some data, commits it, then exits */
void proc1() 
{
     rvm_t rvm;
     trans_t trans;
     char* segs[NUM_SEGS];
	 int i;
     
     rvm = rvm_init("rvm_segments");

	 for(i = 0; i < NUM_SEGS; i++)
	 {
		 char name[50];
		 sprintf(name, "testseg%d", i);
		 rvm_destroy(rvm, name);
		 segs[0] = (char *) rvm_map(rvm, name, 10000);

	     
		 trans = rvm_begin_trans(rvm, 1, (void **) segs);
	     
		 rvm_about_to_modify(trans, segs[i], 0, 100);
		 sprintf(segs[i], TEST_STRING);
	     
		 rvm_about_to_modify(trans, segs[i], OFFSET2, 100);
		 sprintf(segs[i]+OFFSET2, TEST_STRING);
	     
		 rvm_commit_trans(trans);
	 }

     abort();
}


/* proc2 opens the segments and reads from them */
void proc2() 
{
     char* segs[NUM_SEGS];
     rvm_t rvm;
	 int i;
     
     rvm = rvm_init("rvm_segments");
	 
	 for(int i = 0; i < NUM_SEGS; i++)
	 {
		 char name[50];
		 sprintf(name, "testseg%d", i);
		 segs[i] = (char *) rvm_map(rvm, name, 10000);

		 if(i == NUM_SEGS - 1)
		 {
			break;
		 }
		 if(strcmp(segs[i], TEST_STRING)) {
		  printf("ERROR: first hello not present\n");
		  exit(2);
		 }
		 if(strcmp(segs[i]+OFFSET2, TEST_STRING)) {
		  printf("ERROR: second hello not present\n");
		  exit(2);
		 }
	 }
     printf("OK\n");
     exit(0);
}


int main(int argc, char **argv)
{
     int pid;

     pid = fork();
     if(pid < 0) {
	  perror("fork");
	  exit(2);
     }
     if(pid == 0) {
	  proc1();
	  exit(0);
     }

     waitpid(pid, NULL, 0);

     proc2();

     return 0;
}
