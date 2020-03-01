/*
Phase 1b
*/

#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

typedef struct PCB {
    int             cid;                // context's ID
    int             cpuTime;            // process's running time
    char            name[P1_MAXNAME+1]; // process's name
    int             priority;           // process's priority
    P1_State        state;              // state of the PCB
    int            (*func)(void*);
    
    // more fields for profInfo
    //int             sid;
    int             tag;
    int             parent;
    int             children[P1_MAXPROC];
	//int				quitChildren[P1_MAXPROC];
    int             numChildren;

    // extra for P1_Quit
    int             status;

} PCB;

struct Node {
	int data;
	struct Node* next;
};

int i;
struct Node* head = NULL;
struct Node* currNode;
static int running = -1;
static PCB processTable[P1_MAXPROC];   // the process table


int kernelMode(void) {
    int psr = USLOSS_PsrGet();
    if (!(psr & USLOSS_PSR_CURRENT_MODE)) {
        USLOSS_IllegalInstruction();
        P1_Quit(1024);
        return -1;
    }

    return 0;

}

int invalidPid(int pid) {
    if (pid < 0 || pid >= P1_MAXPROC || 
            processTable[pid].state == P1_STATE_FREE) 
        return 1;
    else
        return 0;
}

static void bLaunch(void * arg) {
    
    int r = processTable[running].func(arg);
    P1_Quit(r);
}

// start of function processes ----------------------------------

void P1ProcInit(void)
{
    P1ContextInit();
    for (i = 0; i < P1_MAXPROC; i++) {
        processTable[i].state = P1_STATE_FREE;
		
		// initialize the rest of the PCB
    }
    // initialize everything else

}

int P1_GetPid(void) {
    return running;
}

int P1_Fork(char *name, int (*func)(void*), void *arg, int stacksize, int priority, int tag, int *pid ) 
{
    int result = P1_SUCCESS;
    
    // check for kernel mode
    kernelMode();

    // disable interrupts
    int prevInt = P1DisableInterrupts();
    
    // check all parameters
    if (tag != 0 && tag != 1)
        return P1_INVALID_TAG;
    if (priority < 1 || priority > 6) {
        return P1_INVALID_PRIORITY;
    } 
    if (processTable[0].state != P1_STATE_FREE &&
        priority == 6)
        return P1_INVALID_PRIORITY;
    if (stacksize < USLOSS_MIN_STACK)
        return P1_INVALID_STACK;
    for (i = 0; i < P1_MAXPROC; ++i) {
        if (!strcmp(processTable[i].name, name))
            return P1_DUPLICATE_NAME;
    }
    if (name == NULL)
        return P1_NAME_IS_NULL;
    if (strlen(name) > P1_MAXNAME)
        return P1_NAME_TOO_LONG;
    int flag = 0;
    for (i = 0; i < P1_MAXPROC; ++i) {
        if (processTable[i].state == P1_STATE_FREE) {
            flag = 1; break;
        }
    }
    if (!flag)
        return P1_TOO_MANY_PROCESSES;

    
    // create a context using P1ContextCreate
    int r = P1ContextCreate(&bLaunch, arg, stacksize, pid);
    assert(r == P1_SUCCESS);
    
    PCB *tempPCB = &processTable[*pid];

    // allocate and initialize PCB
    tempPCB->cid = *pid;
    tempPCB->tag = tag;
    tempPCB->cpuTime = 0;
    strcpy(tempPCB->name,name);
    tempPCB->priority = priority;
    tempPCB->state = P1_STATE_READY;
    tempPCB->parent = running;
    memset(tempPCB->children, 0, sizeof(tempPCB->children)); 
    tempPCB->numChildren = 0;
    tempPCB->status = 0;
    tempPCB->func = func;
    

    //add new process to ready queue
   	struct Node* currNode = head;

	if (currNode == NULL) {
		currNode = (struct Node*)malloc(sizeof(struct Node));
		currNode->data = *pid;
		head = currNode;
	}
	else {
    	while (currNode->next != NULL) {
        	currNode = currNode->next;
    	}
		struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
		temp->data = *pid;
		currNode->next = temp;
	}
	

    // if this is the first process or this process's priority is higher than the 
    //    currently running process call P1Dispatch(FALSE)
	//USLOSS_Console("\n\n%s\n\n", processTable[0].name);

	currNode = head;
	int runningLength = 0;
	while (currNode != NULL){
		runningLength++;
		currNode = currNode->next;
	}


	if ( runningLength == 1 || priority < processTable[running].priority){
			P1Dispatch(FALSE);
		}
    // re-enable interrupts if they were previously enabled
    if (prevInt)
        P1EnableInterrupts();
    
    
    return result;
}

void 
P1_Quit(int status) 
{
    // check for kernel mode
	kernelMode();

    // disable interrupts
	int prevInt = P1DisableInterrupts();

    // remove from ready queue
	if (head->next == NULL){
		head = NULL;
	}
	else {
		currNode = head->next;
		struct Node* prevNode = head;

		while (currNode->next != NULL){
			if (currNode->data == running) {
				prevNode->next = currNode->next;
				break;
			}
			prevNode = currNode;
			currNode = currNode->next;
		}
	}


	//set status to P1_STATE_QUIT 
	//(I think they mean set state to P1_STATE_QUIT and set status to parameter)

	processTable[running].status = status;
	processTable[running].state = P1_STATE_QUIT;

    // if first process verify it doesn't have children, otherwise give children to first process

	if (running == 0 && processTable[running].numChildren != 0) { 
		USLOSS_Console("First process quitting with children, halting.");
        USLOSS_Halt(1);
	}
	else if (running != 0){
		for (i = 0; i < P1_MAXPROC; i++){
			if (processTable[running].children[i] == 1){
				processTable[0].children[i] = 1;
                processTable[i].parent = 0;
			}
		}
	}


    /*-----------------------------------------------
	int parent = processTable[running].parent;

    // add ourself to list of our parent's children that have quit
	processTable[parent].quitChildren[running] = 1;

    // if parent is in state P1_STATE_JOINING set its state to P1_STATE_READY
	if (processTable[parent].state == P1_STATE_JOINING) {
		processTable[parent].state = P1_STATE_READY;
	}

	if (prevInt)
        P1EnableInterrupts();

    */

    P1Dispatch(FALSE);
    // should never get here
    assert(0);
}


int 
P1GetChildStatus(int tag, int *pid, int *status) 
{
    kernelMode();

    //current running process
    PCB *cur = &processTable[running]; 
    
    //checking for valid tags
    if (tag != 0 && tag != 1)
        return P1_INVALID_TAG;

    //check for children count
    if (cur->numChildren == 0)
        return P1_NO_CHILDREN;

    int no_quit = 0; //flag for checking type of return
    for (i = 0; i < P1_MAXPROC; ++i) {

        //getting the child
        if(cur->children[i]) {

            PCB *child = &processTable[i];
            if (child->tag == tag) {
                
                if (child->state == P1_STATE_QUIT) {

                    //child's tag match and state is quit
                    *pid = i;
                    *status = child->status;
                    int r = P1ContextFree(i);
					assert(r = P1_SUCCESS);
                    return P1_SUCCESS;

                } else {
                    /* if we have child with matching tag but its
                     * state is not QUIT, it means we have a child
                     * that has not quit, AKA P1_NO_QUIT
                    */
                    no_quit = 1;
                }
            }
        }
    }

    // at this point we have only unsuccesful returns

    if (no_quit)
        return P1_NO_QUIT;
    
    return P1_NO_CHILDREN;

}

int
P1SetState(int pid, P1_State state, int sid) 
{
    kernelMode();

    if(invalidPid(pid))
        return P1_INVALID_PID;

    if (processTable[pid].state == P1_STATE_JOINING) {
        for (i = 0; i < P1_MAXPROC; ++i) {
            int child = processTable[pid].children[i];
            if (child) {
                if (processTable[i].state == P1_STATE_QUIT)
                    return P1_CHILD_QUIT;
            }
        }
    }
    
    if (state == P1_STATE_READY ||
		state == P1_STATE_JOINING ||
		state == P1_STATE_BLOCKED ||
		state == P1_STATE_QUIT) {
        
		processTable[pid].state = state;
    } 
	
	else {
        return P1_INVALID_STATE;
    }


    return P1_SUCCESS;

}

void
P1Dispatch(int rotate)
{
    // select the highest-priority runnable process
    int flag = 0;

	currNode = head;
	int runningLength = 0;

    // should be removed 
	while (currNode != NULL){
		runningLength++;
		currNode = currNode->next;
	}
    //--------------------------------


	currNode = head;


    // find the lowest priority and also the first same priority from the current one.
    // change thi while loop to fill criteria
    while (currNode != NULL) {
        if (processTable[currNode->data].state == P1_STATE_READY) {
            if (processTable[currNode->data].priority < processTable[running].priority ||
				/*remove*/runningLength == 1) {
					

				//processTable[running].state = P1_STATE_READY;
                if (processTable[running].state == P1_STATE_RUNNING)
                    processTable[running].state == P1_STATE_READY;
				running = currNode->data;
				processTable[currNode->data].state = P1_STATE_RUNNING;
                int r = P1ContextSwitch(currNode->data);
                assert(r == P1_SUCCESS);
                return P1_SUCCESS;
            }

            if (rotate == TRUE) {
                if (currNode->data != running && 
                        processTable[currNode->data].priority == processTable[running].priority) {
					processTable[running].state = P1_STATE_READY;
					running = currNode->data;
					processTable[currNode->data].state = P1_STATE_RUNNING;
                    int r = P1ContextSwitch(currNode->data);
                    assert(r == P1_SUCCESS);
                    return P1_SUCCESS;
                }
            }
            flag = 1;
        }
		currNode = currNode->next;
    }

    if (!flag) {
        USLOSS_Console("No runnable proccesses, halting.");
        USLOSS_Halt(0);
    }




    // call P1ContextSwitch to switch to that process
}

int
P1_GetProcInfo(int pid, P1_ProcInfo *info)
{
    kernelMode();

    if (pid < 0 || pid >= P1_MAXPROC || 
            processTable[pid].state == P1_STATE_FREE)
        return P1_INVALID_PID;
    
    PCB *process = &processTable[pid];

    strcpy(info->name, process->name);
    info->state       = process->state;
    //info->sid         = process->sid;
    info->priority    = process->priority;
    info->tag         = process->tag;
    info->cpu         = process->cpuTime;
    info->parent      = process->parent;
    for (i = 0; i < P1_MAXPROC; ++i) {
        info->children[i] = process->children[i];
    }  

    info->numChildren = process->numChildren;
    
    return P1_SUCCESS;
}

