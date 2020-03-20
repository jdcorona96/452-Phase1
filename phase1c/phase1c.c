
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

// node for the Linked List
struct node {
	int pid;
  	struct node *next;
};
  
typedef struct Sem
{
    char        name[P1_MAXNAME+1];
    u_int       value;
    
    // more fields here 
    struct node *head; // for the sem waiting Queue
    struct node *rdy; // for the sem ready queue
    int  rdySize;

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
        
        // setting head of the sem queue
        struct node *headInit = (struct node*) malloc(sizeof(struct node)); 
        headInit->pid = -1; // always NULL
        headInit->next = NULL; //setting it as an empty linked list
        sems[i].head = headInit;

        struct node *rdyInit = (struct node*) malloc(sizeof(struct node)); 
        rdyInit->pid = -1; // always NULL
        rdyInit->next = NULL; //setting it as an empty linked list
        sems[i].rdy = rdyInit;

        sems[i].rdySize = 0;


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

        //setting values of sem
      strcpy(sems[i].name, name);
      sems[i].value = value;

      flag = 0;
      *sid = i;
      break;
    }
  }
  
  // P1_TOO_MANY_SEMS: no more semaphores
  if (flag)
    return P1_TOO_MANY_SEMS;
  
  if (prevInt) 
    P1EnableInterrupts();

  return P1_SUCCESS;
}

int P1_SemFree(int sid)
{
    // P1_INVALID_SID: the semaphore is invalid
  if (sid < 0 || sid > P1_MAXSEM)
    return P1_INVALID_SID;

  struct node *head = sems[sid].head;

  // P1_BLOCKED_PROCESSES: processes are blocked on the semaphore
  if (head->next != NULL) { // this process has blocked processes
      return P1_BLOCKED_PROCESSES;
  }
  
  // freeing process
  sems[sid].name[0] = '\0';

  struct node *rdy = sems[sid].rdy;
  while(rdy->next != NULL) {

      struct node *temp = rdy->next;
      rdy->next = rdy->next->next;
      free(temp);
  }

  sems[sid].rdySize = 0;

  return P1_SUCCESS; 
}

int P1_P(int sid)
{
    kernelMode();

    if (sid < 0 || sid > P1_MAXSEM)
        return P1_INVALID_SID;
  
    int prevInt = P1DisableInterrupts();
    struct node *head = sems[sid].head; 

    int value = sems[sid].value;
    int rdySize = sems[sid].rdySize;

    if (head->next != NULL || value <= rdySize ) {
    
        // setting the process to the end of the waiting queue
        struct node *ite = sems[sid].head; 
        while (ite->next != NULL) {
            ite = ite->next;
        }
        struct node *blkProcess = (struct node*) malloc(sizeof(struct node));
        blkProcess->pid = P1_GetPid();
        blkProcess->next = NULL;
        
        ite->next = blkProcess;

        // loops until process has a resource available
        while(sems[sid].value == 0) {
     
            int runningPid = blkProcess->pid;
            int rt = P1SetState(runningPid, P1_STATE_BLOCKED, sid);
            if (rt != P1_SUCCESS)
                USLOSS_Halt(1);
        
            // dispatch to other process
            P1Dispatch(FALSE);
            if (rt != P1_SUCCESS)
                USLOSS_Halt(1);
        }

        struct node *ready = sems[sid].rdy;
        while (ready->next != NULL) {
            
            if (ready->next->pid == P1_GetPid()) {

                struct node *temp = ready->next;
                ready->next = temp->next;
                free(temp);
                sems[sid].rdySize--;
                break;
            }
        }
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

    if (sid < 0 || sid > P1_MAXSEM)
        return P1_INVALID_SID;
  
    int prevInt = P1DisableInterrupts();
  
    // frees resource
    sems[sid].value++;
    struct node *head = sems[sid].head;
  
    // sets first element in linked list to ready queue
    if (head->next != NULL) {

        struct node *headElem = head->next;
	    
        int nextPid = headElem->pid;
        int rt = P1SetState(nextPid, P1_STATE_READY, sid);

        head->next = headElem->next;
        
        struct node *rdy = sems[sid].rdy;
        headElem->next = rdy->next;
        rdy->next = headElem;
        sems[sid].rdySize++;

        if (rt != P1_SUCCESS)
            USLOSS_Halt(1);
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


