/* A simple asynchronous XML-RPC client written in C, as an example of
   Xmlrpc-c asynchronous RPC facilities.  This is the same as the 
   simpler synchronous client xmlprc_sample_add_client.c, except that
   it adds 3 different pairs of numbers with the summation RPCs going on
   simultaneously.

   Use this with xmlrpc_sample_add_server.  Note that that server
   intentionally takes extra time to add 1 to anything, so you can see
   our 5+1 RPC finish after our 5+0 and 5+2 RPCs.
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#include "config.h"  /* information about this build environment */

#define NAME "Xmlrpc-c Asynchronous Test Client"
#define VERSION "1.0"
#define SEMANTIC_ANY 1
#define SEMANTIC_MAJORITY 2
#define SEMANTIC_ALL 3	
#define TOTAL_SERVERS 3	


//#define ASYNC


int numResults;
xmlrpc_int sum;
int synchronous;
int handling_semantic;
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

static void 
die_if_fault_occurred (xmlrpc_env * const envP) {
  if (envP->fault_occurred) {
    fprintf(stderr, "Something failed. %s (XML-RPC fault code %d)\n",
	    envP->fault_string, envP->fault_code);
    exit(1);
  }
}



static void 
handle_sample_add_response(const char *   const server_url,
                           const char *   const method_name,
                           xmlrpc_value * const param_array,
                           void *         const user_data ATTR_UNUSED,
                           xmlrpc_env *   const faultP,
                           xmlrpc_value * const resultP) {
  
  xmlrpc_env env;
  xmlrpc_int addend, adder;
  
  /* Initialize our error environment variable */
  xmlrpc_env_init(&env);
  
  /* Our first four arguments provide helpful context.  Let's grab the
     addends from our parameter array. 
  */
  xmlrpc_decompose_value(&env, param_array, "(ii)", &addend, &adder);
  die_if_fault_occurred(&env);
  
  printf("RPC with method '%s' at URL '%s' to add %d and %d "
	 "has completed\n", method_name, server_url, addend, adder);
  numResults++;
  if (faultP->fault_occurred)
    printf("The RPC failed.  %s", faultP->fault_string);
  else {
    
    xmlrpc_read_int(&env, resultP, &sum);
    die_if_fault_occurred(&env);
    
    printf("\nSemantic: %d numResults: %d", handling_semantic, numResults);
	
    int complete = 0;
    if (handling_semantic == SEMANTIC_ANY && numResults == 1)
      {
	complete = 1;
	printf("\nThe sum is  %d\n", sum);
      }
    else if (handling_semantic == SEMANTIC_MAJORITY)
      {
	complete = (numResults > TOTAL_SERVERS / 2);
      }
    else if (handling_semantic == SEMANTIC_ALL)	
      {
	complete = (numResults == TOTAL_SERVERS);
      }
    if(complete) 
      {
	if(synchronous == 1)
	  {
	    pthread_mutex_unlock(&mymutex);
	    printf("\nunlocking mutex");	
	  }
	else
	  {
	    ;//invoke "real" handler
	  }
      }
  }
}

void call_rpc_asynchronous(xmlrpc_client * clientP)
{
  char* serverUrls[1024];  
  serverUrls[0] = "http://localhost:8080/RPC2";
  serverUrls[1] = "http://localhost:8081/RPC2";
  serverUrls[2] = "http://localhost:8082/RPC2";
  
  char * const methodName = "sample.add";
  
  xmlrpc_env env;
  xmlrpc_int adder;

    /* Initialize our error environment variable */
  xmlrpc_env_init(&env);
 
  for (adder = 0; adder < 3; ++adder) {
    printf("Making XMLRPC call to server url '%s' method '%s' "
	   "to request the sum "
	   "of 5 and %d...\n", serverUrls[adder], methodName, 3);
    
    /* request the remote procedure call */
    xmlrpc_client_start_rpcf(&env, clientP, serverUrls[adder], methodName,
			     handle_sample_add_response, NULL,
			     "(ii)", (xmlrpc_int32) 5, 3);
    die_if_fault_occurred(&env);
  }
}

int 
main(int           const argc, 
     const char ** const argv ATTR_UNUSED) {
  
  numResults = 0;
  sum = 0;
  synchronous = 1;
  handling_semantic = SEMANTIC_ANY;
  xmlrpc_client * clientP;

  if (argc-1 > 0) {
    fprintf(stderr, "This program has no arguments\n");
    exit(1);
  }   
  
   xmlrpc_env env;
  /* Initialize our error environment variable */
  xmlrpc_env_init(&env);
  
  /* Required before any use of Xmlrpc-c client library: */
  xmlrpc_client_setup_global_const(&env);
  die_if_fault_occurred(&env);
  
  xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0,
		       &clientP);
  die_if_fault_occurred(&env);

  if(synchronous == 1)
    {
      pthread_mutex_lock(&mymutex);
      call_rpc_asynchronous(clientP);
      printf("RPCs all requested.  Waiting for & handling responses...\n");
      pthread_mutex_lock(&mymutex);
    }
    
    printf("\n GOT RESPONSE: %d", sum);

  /* Wait for all RPCs to be done.  With some transports, this is also
     what causes them to go.
  */
  xmlrpc_client_event_loop_finish(clientP);
  
  printf("All RPCs finished.\n");
  
  xmlrpc_client_destroy(clientP);
  
  return 0;
}
