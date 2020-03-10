
/* Phase1c project
 * Joseph Corona | jdcorona96
 * Luke | lacernetic
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "usloss.h"
#include "phase1Int.h"

// might not be used ----------------------------------------------------------------------------//intid
struct node {
	int val;
    PCB process;
  	struct node *next;
}
// ----------------------------------------------------------------------------------------------  
  
typedef struct Sem
{
    char        name[P1_MAXNAME+1];
    u_int       value;
    
  // more fields here
	  struct node *head;
} Sem;

static Sem sems[P1_MAXSEM];

void kernelMode(void) {

  int psr = USLOSS_PsrGet();
  if (!(psr & USLOSS_PSR_CURRENT_MODE)) {     
    USLOSS_IllegalInstruction();

  }
}


void 
P1SemInit(void)
{
    P1ProcInit();
    for (int i = 0; i < P1_MAXSEM; i++) {
        sems[i].name[0] = '\0';
        // initialize rest of sem here
    }
}

int P1_SemCreate(char *name, unsigned int value, int *sid)
{
  kernelMode();

  int prevInt = P1DisableInterrupts();
  
  // P1_NAME_IS_NULL: name is NULL
  if (name == NULL)
    return P1_NAME_IS_NULL;
  
  // P1_DUPLICATE_NAME: name already in use
  int i;
  for (i =0; i < P1_MAXSEM; ++i) {
    if (!strcmp(name, sems[i].name))
      return P1_DUPLICATE_NAME;
  }
  
  // P1_NAME_TOO_LONG: name is longer than P1_MAXNAME
  if (strlen(name) > P1_MAXNAME)
    return P1_NAME_TOO_LONG;
    
  int flag = 1;
  for (i = 0; i < P1_MAXSEM; ++i) {
    if (sems[i].name[0] == '\0') {
      strcpy(sems[i].name, name);
      sems[i].value = value;
      // might be deleted ---------------------------------------------------------------------------
      struct node *headInit = (struct node*) malloc(sizeof(struct node)); 
      headInit->pid = -1; // always NULL
      headInit->next = NULL; //setting it as an empty linked list
      // ---------^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

      sems[i].head = headInit; //this too ^^
      flag = 0;
      *sid = i;
      break;
    }
  }
  
  // P1_TOO_MANY_SEMS: no more semaphores
  if (flag)
    return P1_TOO_MANY_SEMS;
  
  if (prevInt) 
    P1EnableInterrupts;

  P1_SUCCESS
}

int P1_SemFree(int sid)
{
    // P1_INVALID_SID: the semaphore is invalid
  if (sid < 0 || sid > P1_MAXSEM)
    return P1_INVALID_SID;

  // might need to be changed -------------------------------------------------------------------------
  // P1_BLOCKED_PROCESSES: processes are blocked on the semaphore
  if (sems[sid].head.next.next != NULL)
    return P1_BLOCKED_PROCESSES;
  // ----------------------------------------------------^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
  // freeing process
  free(sems[sid].head); //TODO might need to change it
  sems[sid].name[0] = '\0';

  return P1_SUCCESS; 
}

int P1_P(int sid)
{
  kernelMode()
  int prevInt = P1DisableInterrupts();
  
  // while value == 0
  //      set state to P1_STATE_BLOCKED
  while(sems[sid].value == 0) {
    
    int runningPCB = P1_GetPid();
    P1SetState(runningPCB, P1_STATE_BLOCKED, sid);
    //processTable[runningPCB].state = P1_STATE_BLOCKED;
  }
  
  // value--
  sems[sid].value--;
  
  if (prevInt)
    P1EnableInterrupts();
   return P1_SUCCESS;
}

int P1_V(int sid)
{
  kernelMode();
  
  int prevInt = P1DisableInterrupts();
  
  sems[sid].value++;
    struct node *head = sems[sid].head;
  
  if (head->next->next != NULL) {
	struct node *blocked = head->next->next;
    blocked->process.state = P1_STATE_READY;
  }
    
  // re-enable interrupts if they were previously enabled
  if (prevInt)
    P1EnableInterrupts();
    return P1_SUCCESS;  
}

int P1_SemName(int sid, char *name) {
     	char* semName = sems[sid].name;
  	strcpy(name, semName);
	return P1_SUCCESS;
}


