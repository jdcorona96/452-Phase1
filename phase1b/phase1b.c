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
    
    // more fields for profInfo
    //int             sid;
    //int             tag;
    int             parent;
    int             children[P1_MAXPROC];
    int             numChildren;

    // extra for P1_Quit
    int             status;

} PCB;

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
    
    P1_Quit(11);
}

// start of function precesses ----------------------------------

void P1ProcInit(void)
{
    P1ContextInit();
    for (int i = 0; i < P1_MAXPROC; i++) {
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
    if (tag != 0 || tag != 1)
        return P1_INVALID_TAG;
    if (priority < 1 || priority > 6) {
        return P1_INVALID_PRIORITY;
    } 
    if (processTable[0].state != P1_STATE_FREE &&
        priority == 6)
        return P1_INVALID_PRIORITY;
    if (stacksize < USLOSS_MIN_STACK)
        return P1_INVALID_STACK;
    int i;
    for (i =0; i < P1_MAXPROC; ++i) {
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
    P1ContextCreate(func, arg, stacksize, pid);
    
    PCB *tempPCB = &processTable[*pid];

    // allocate and initialize PCB
    tempPCB->cid = *pid;
    tempPCB->cpuTime = 0;
    tempPCB->name = name;
    tempPCB->priority = priority;
    tempPCB->state = P1_STATE_READY;
    tempPCB->parent = running;
    memset(tempPCB->children, 0, sizeof(tempPCB->children)); 
    tempPCB->numChildren = 0;
    tempPCB->status = 0;

    // if this is the first process or this process's priority is higher than the 
    //    currently running process call P1Dispatch(FALSE)
    // re-enable interrupts if they were previously enabled
    return result;
}

void 
P1_Quit(int status) 
{
    // check for kernel mode
    // disable interrupts
    // remove from ready queue, set status to P1_STATE_QUIT
    // if first process verify it doesn't have children, otherwise give children to first process
    // add ourself to list of our parent's children that have quit
    // if parent is in state P1_STATE_JOINING set its state to P1_STATE_READY
    P1Dispatch(FALSE);
    // should never get here
    assert(0);
}


int 
P1GetChildStatus(int tag, int *cpid, int *status) 
{
    int result = P1_SUCCESS;
    // do stuff here
    return result;
}

int
P1SetState(int pid, P1_State state, int sid) 
{
    kernelMode();

    if(invalidPid(pid))
        return P1_INVALID_PID;

    if (processTable[pid].state == P1_STATE_JOINING) {
        int i;
        for (i = 0; i < P1_MAXPROC; ++i) {
            int child = processTable[pid].children[i];
            if (child) {
                if (processTable[i].state == P1_STATE_QUIT)
                    return P1_CHILD_QUIT;
            }
        }
    }
    
    if (state == P1_STATE_READY) {
        processTable[pid].state = state;
        return P1_SUCCESS;

    } else if (state == P1_STATE_JOINING) {
        processTable[pid].state = state;
        return P1_SUCCESS;

    } else if (state == P1_STATE_BLOCKED) {
        processTable[pid].state = state;
        // sid is the ID of the semaphore on which it is blocked
        return P1_SUCCESS;

    } else if (state == P1_STATE_QUIT) {
        processTable[pid].state = state;
        return P1_SUCCESS;

    } else {
        return P1_INVALID_STATE;
    }


    return P1_SUCCESS;

}

void
P1Dispatch(int rotate)
{
    // select the highest-priority runnable process
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
    //info->tag         = process->tag;
    info->cpu         = process->cpuTime;
    info->parent      = process->parent;
    for (int i = 0; i < P1_MAXPROC; ++i) {
        info->children[i] = process->children[i];
    }  

    info->numChildren = process->numChildren;
    
    return P1_SUCCESS;
}

