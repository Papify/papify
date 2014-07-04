/* Standard headers for PAPI test applications.
	This file is customized to hide Windows / Unix differences.
*/

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>
#if (!defined(NO_DLFCN) && !defined(_BGL) && !defined(_BGP))
#include <dlfcn.h>
#endif

#include <errno.h>
#include <memory.h>
#if !defined(__FreeBSD__) && !defined(__APPLE__)
#include <malloc.h>
#endif
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
//#include <math.h>

#include <pthread.h>
#include "papiStdEventDefs.h"
#include <papi.h>

#define RED    "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN  "\033[1;32m"
#define NORMAL "\033[0m"


#define OVER_FMT    "handler(%d ) Overflow at %p! bit=0x%llx \n"


/*  Variable to hold reporting status
	if TRUE, output is suppressed
	if FALSE output is sent to stdout
	initialized to FALSE
	declared here so it can be available globally
*/
//int TESTS_QUIET = 0;
static int TESTS_COLOR = 0;
//static int TEST_WARN = 0;

//STRUCTURES
typedef struct papi_action_s {
	char *action_id;
	int eventCodeSetSize;
	int *eventCodeSet; //size = eventCodeSetSize
	unsigned long *eventSet; //size = eventCodeSetSize
	unsigned long *counterValues; //size = eventCodeSetSize
} papi_action_s;


/* OLD:
typedef struct papi_eventValues_s { //counters
	int eventValuesSize;
	unsigned long *values[];
} papi_eventValues_s;

typedef struct papi_eventSet_s {
	int eventSetSize;
	papi_eventValues_s *values;
	unsigned long *eventSets[];//I'll have as many eventsets as action_nb
} papi_eventSet_s;

typedef struct papi_eventCodeSet_s {
	int *eventCodeSetSize;
	int *eventCodeSets[];
} papi_eventCodeSet_s;

typedef struct papi_action_s {
	int action_nb;
	papi_eventCodeSet_s *eventCodeSet;
	char *action_id[];
} papi_action_s;
/*
typedef struct papi_actor_s{
	int actor_nb;
	papi_action_s *actions;
	//char *actors_src_path[];
} papi_actor_s;*/
/*
typedef struct papi_thread_s {
	int thread_nb;
	papi_actor_s actors;
	int thread_id[];
} papi_thread_s;

typedef struct papi_project_s {
	char *project_path;
	papi_thread_s threads;
} papi_project_s;
*/

