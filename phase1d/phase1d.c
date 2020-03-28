#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "usloss.h"
#include "phase1.h"
#include "phase1Int.h"

static void DeviceHandler(int type, void *arg);
static void SyscallHandler(int type, void *arg);
static void IllegalInstructionHandler(int type, void *arg);

static int sentinel(void *arg);

// Globabl variables
Sem devSem[4][4]; 

//------------

// initializes all global variables
static void intInit() {

    int sid;

    //TODO: check which value each sem must start

    P1_SemCreate("Clock", 1, &sid);
    devSem[0][0] =  sid;
    P1_SemCreate("Alarm", 1, &sid);
    devSem[1][0] =  sid;
    P1_SemCreate("Disk0", 1, &sid);
    devSem[2][0] =  sid;
    P1_SemCreate("Disk1", 1, &sid);
    devSem[2][1] =  sid;
    P1_SemCreate("Term0", 1, &sid);
    devSem[3][0] =  sid;
    P1_SemCreate("Term1", 1, &sid);
    devSem[3][1] =  sid;
    P1_SemCreate("Term2", 1, &sid);
    devSem[3][2] =  sid;
    P1_SemCreate("Term3", 1, &sid);
    devSem[3][3] =  sid;

}

void kernelMode(void) {
    int psr = USLOSS_PsrGet();
    
    if (!(psr & USLOSS_PSR_CURRENT_MODE)) {
        USLOSS_IllegalInstruction();   
    }    
}


void 
startup(int argc, char **argv)
{
    int pid;
    P1SemInit();

    // initialize device data structures
    intInit();

    // put device interrupt handlers into interrupt vector
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = SyscallHandler;
    //TODO: more to do? deviceHandler() ?

    /* create the sentinel process */
    int rc = P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6 , 0, &pid);
    assert(rc == P1_SUCCESS);
    // should not return
    assert(0);
    return;

} /* End of startup */

int 
P1_WaitDevice(int type, int unit, int *status) 
{
    // disable interrupts
    int prevInt = P1DisableInterrupts();

    // check kernel mode
    kernelMode();

    // cehking if type is valid
    if (type < 0 || type > 3)
        return P1_INVALID_TYPE;
    
    // checking if unit is valid based on type
    if (type == 0 || type == 1) {
        if (unit != 0)
            return P1_INVALID_UNIT;
    }
    if (type == 2) {
        if (unit != 0 && unit != 1)
            return P1_INVALID_UNIT;
    }
    if (type == 3) {
        if (unit < 0 || unit > 3)
            return P1_INVALID_UNIT;
    }

    // p device's semaphore
    P1_P(devsem[type][unit]);

    // set *status to device's status
    // TODO: how to get status??
    // suggestion: set up global variables to store more info on 
    //             each interrupt handler
    
    // restore interrupts
    if (prevInt)
        P1EnableInterrupts();

    return P1_SUCCESS;
}


int 
P1_WakeupDevice(int type, int unit, int status, int abort) 
{

    // disable interrupts
    int prevInt = P1DisableInterrupts();

    // check kernel mode
    kernelMode();

    // cehking if type is valid
    if (type < 0 || type > 3)
        return P1_INVALID_TYPE;
    
    // checking if unit is valid based on type
    if (type == 0 || type == 1) {
        if (unit != 0)
            return P1_INVALID_UNIT;
    }
    if (type == 2) {
        if (unit != 0 && unit != 1)
            return P1_INVALID_UNIT;
    }
    if (type == 3) {
        if (unit < 0 || unit > 3)
            return P1_INVALID_UNIT;
    }

    // save device's status to be used by p1_waitdevice
    // save abort to be used by p1_waitdevice

    // v device's semaphore
    P1_V(devsem[type][unit]);

    // restore interrupts
    if (prevInt)
        P1EnableInterrupts();

    return P1_SUCCESS;

}

static void
DeviceHandler(int type, void *arg) 
{
    // if clock device
    //      p1_wakeupdevice every 5 ticks
    //      p1dispatch(true) every 4 ticks
    // else
    //      p1_wakeupdevice
}

static int
sentinel (void *notused)
{
    int     pid;
    int     rc;
	int 	status;
	p1_procinfo info;


    /* start the p2_startup process */
    rc = P1_Fork("P2_Startup", P2_Startup, null, 4 * USLOSS_MIN_STACK, 2 , 0, &pid);
    assert(rc == P1_SUCCESS);

    // enable interrupts
    P1EnableInterrupts();

	rc = P1_GetProcInfo(pid, info);
	assert(rc == P1_SUCCESS);
	while (info->numChildren != 0){

		
		rc = P1GetChildStatus(0, pid, status);
		
		// if a child exists
		if (rc == P1_NO_QUIT){
			USLOSS_WaitInt();
		}
		else {
			//check other tag
			rc = P1GetChildStatus(1, pid, status);

			//if a child exists
			if (rc == P1_NO_QUIT) {
				USLOSS_WaitInt();
			}
		}

		rc = P1_GetProcInfo(pid, info);
		assert(rc == P1_SUCCESS);
	}


    // while sentinel has children
    // phase1b: procinfo - get number of children

    //      get children that have quit via p1getchildstatus (either tag)
    //      wait for an interrupt via usloss_waitint
	

	// no children left
    USLOSS_Console("sentinel quitting.\n");
    return 0;
} /* end of sentinel */

int 
P1_Join(int tag, int *pid, int *status) 
{
	// disable interrupts
    int prevint = P1DisableInterrupts();

    // check kernel mode
    kernelMode();



	int pid = P1_GetPid();
	P1_ProcInfo info;

	rc = P1_GetProcInfo(pid, info);
	assert(rc == P1_SUCCESS);

	int i;

	// for each child                        
	for (i = 0; i < info->numChildren; i++){

		// get child info through getChildStatus
		rc = P1GetChildStatus(tag, *pid, *status);
	}
    // do
    //     use P1GetChildStatus to get a child that has quit  
    //     if no children have quit
    //        set state to P1_STATE_JOINING vi P1SetState
    //        P1Dispatch(FALSE)
    // until either a child quit or there are no more children
    return P1_SUCCESS;
}

static void
SyscallHandler(int type, void *arg) 
{
    USLOSS_Console("System call %d not implemented.\n", (int) arg);
    USLOSS_IllegalInstruction();
}

void finish(int argc, char **argv) {}
