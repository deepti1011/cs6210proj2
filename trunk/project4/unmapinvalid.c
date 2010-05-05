/* basic.c - test that basic persistency works */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"
#define OFFSET2 1000
#define NUM_SEGS 10

/* proc1 writes some data, commits it, then exits */
void proc1() 
{
     rvm_t rvm;
     trans_t trans;
     char* segs[NUM_SEGS];
	 int i;
     
     rvm = rvm_init("rvm_segments");
	 segs[0] = (char *) rvm_map(rvm, "testseg0", 10000);
	 //segs[1] = (char *) rvm_map(rvm, "testseg1", 10000);
	 segs[2] = (char *) rvm_map(rvm, "testseg2", 10000);
	 segs[3] = (char *) rvm_map(rvm, "testseg3", 10000);
	 segs[4] = (char *) rvm_map(rvm, "testseg4", 10000);
	 segs[5] = (char *) rvm_map(rvm, "testseg5", 10000);
	 segs[6] = (char *) rvm_map(rvm, "testseg6", 10000);
	 //segs[7] = (char *) rvm_map(rvm, "testseg7", 10000);
	 segs[8] = (char *) rvm_map(rvm, "testseg8", 10000);
	 segs[9] = (char *) rvm_map(rvm, "testseg9", 10000);

	 rvm_unmap(rvm, segs[0]);
	 rvm_unmap(rvm, segs[1]);
	 rvm_unmap(rvm, segs[2]);
	 rvm_unmap(rvm, segs[3]);
	 rvm_unmap(rvm, segs[4]);
	 rvm_unmap(rvm, segs[5]);
	 rvm_unmap(rvm, segs[6]);
	 rvm_unmap(rvm, segs[7]);
	 rvm_unmap(rvm, segs[8]);
	 rvm_unmap(rvm, segs[9]);

	 printf("OK, Should see 2 handled exceptions above.\n");
}

int main(int argc, char **argv)
{
     proc1();

     return 0;
}
