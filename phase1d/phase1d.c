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

typedef struct DeviceInfo {
	int sid;
	int status;
	int abort;
}

// Global variables
Sem devSem[4][4]; 
DeviceInfo deviceArray[4][4];
int ticks = 0;

//------------

// initializes all global variables
static void intInit() {

    int sid;
    P1_SemCreate("Clock", 1, &sid);
    deviceArray[0][0].sid = sid;
	deviceArray[0][0].status = 0;
	deviceArray[0][0].aborted = 0;

    P1_SemCreate("Alarm", 1, &sid);
    deviceArray[1][0].sid = sid;
	deviceArray[1][0].status = 0;
	deviceArray[1][0].aborted = 0;

    P1_SemCreate("Disk0", 1, &sid);
    deviceArray[2][0].sid = sid;
	deviceArray[2][0].status = 0;
	deviceArray[2][0].aborted = 0;

    P1_SemCreate("Disk1", 1, &sid);
    deviceArray[2][1].sid = sid;
	deviceArray[2][1].status = 0;
	deviceArray[2][1].aborted = 0;

    P1_SemCreate("Term0", 1, &sid);
    deviceArray[3][0].sid = sid;
	deviceArray[3][0].status = 0;
	deviceArray[3][0].aborted = 0;

    P1_SemCreate("Term1", 1, &sid);
    deviceArray[3][1].sid = sid;
	deviceArray[3][1].status = 0;
	deviceArray[3][1].aborted = 0;

    P1_SemCreate("Term2", 1, &sid);
    deviceArray[3][2].sid = sid;
	deviceArray[3][2].status = 0;
	deviceArray[3][2].aborted = 0;

    P1_SemCreate("Term3", 1, &sid);
    deviceArray[3][3].sid = sid;
	deviceArray[3][3].status = 0;
	deviceArray[3][3].aborted = 0;


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
	USLOSS_IntVec[USLOSS_CLOCK_INT] = SyscallHandler;
	USLOSS_IntVec[USLOSS_ALARM_INT] = SyscallHandler;
	USLOSS_IntVec[USLOSS_TERM_INT] = SyscallHandler;
	USLOSS_IntVec[USLOSS_DISK_INT] = SyscallHandler;

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

	if (deviceArray[type][unit].abort == 1) {
		return P1_WAIT_ABORTED;
	}

    // checking if type is valid
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

	*status = deviceArray[type][unit].status;
 
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

	deviceArray[type][unit]->status = status;
	deviceArray[type][unit]->abort = abort;

    // restore interrupts
    if (prevInt)
        P1EnableInterrupts();

    return P1_SUCCESS;

}

static void
DeviceHandler(int type, void *arg) 
{
	int *status;

	int rc = USLOSS_DeviceInput(type, *arg, status);
	assert(rc == USLOSS_DEV_OK);

	if (type == 0){
		ticks++;
		if (ticks % 5 == 0) {
			rc = P1_WakeupDevice(type, arg, *status, 0);
			assert(rc == P1_SUCCESS);
		}

		if (ticks % 4 == 0) {
			P1Dispatch(TRUE);
		}
	}
	else {
		rc = P1_WakeupDevice(type, arg, *status, 0);
		assert(rc == P1_SUCCESS);
	}
    // if clock device
    //      p1_wakeupdevice every 5 ticks
    //      p1dispatch(true) every 4 ticks
    // else
    //      p1_wakeupdevice
}

static int
sentinel (void *notUsed)
{
    int     pid;
    int     rc;
	int 	status;
	P1_ProcInfo info;


    /* start the p2_startup process */
    rc = P1_Fork("P2_Startup", P2_Startup, null, 4 * USLOSS_MIN_STACK, 2 , 0, &pid);
    assert(rc == P1_SUCCESS);

    // enable interrupts
    P1EnableInterrupts();


	int currPid = P1_GetPid();
	

    // while sentinel has children
	while (info->numChildren != 0){
		
		// phase1b: procinfo - get number of children
		rc = P1_GetProcInfo(currPid, info);
		assert(rc == P1_SUCCESS);
		
		//check for children with tag 0
		rc = P1GetChildStatus(0, currPid, status);
		
		// if a child exists
		if (rc == P1_NO_QUIT){
			USLOSS_WaitInt();
		}
		else {
			//check for children with tag 1
			rc = P1GetChildStatus(1, currPid, status);

			//if a child exists
			if (rc == P1_NO_QUIT) {
				USLOSS_WaitInt();
			}
		}
	}


    // while sentinel has children
    //      get children that have quit via p1getchildstatus (either tag)
    //      wait for an interrupt via usloss_waitint
	

	// no children left
    USLOSS_Console("Sentinel quitting.\n");
    return 0;
} /* end of sentinel */

int 
P1_Join(int tag, int *pid, int *status) 
{
	// disable interrupts
    int prevInt = P1DisableInterrupts();

    // check kernel mode
    kernelMode();

	if (tag != 1 && tag != 0) {
		return P1_INVALID_TAG;
	}

	int pid = P1_GetPid();
	P1_ProcInfo info;

	rc = P1_GetProcInfo(pid, info);
	assert(rc == P1_SUCCESS);


	int returnPid;
	int returnStatus; 

	// while there are children                        
	do {	

		// get child info through getChildStatus
		rc = P1GetChildStatus(tag, &returnPid, &returnStatus);
		if (rc == P1_NO_QUIT){ 
			rc = P1SetState(pid, P1_STATE_JOINING, info.sid)
			assert(rc == P1_SUCCESS);
			P1Dispatch(FALSE);

			//get current number of children
			rc = P1_GetProcInfo(pid, info);
			assert(rc == P1_SUCCESS);

		}
		else if (rc == P1_NO_CHILDREN){
			return P1_NO_CHILDREN;
		}
		
		//child has quit
		else if (rc == P1_SUCCESS){
			break;
		}
	} while (info->numChildren != 0);	

	*pid = returnPid;
	*status = returnStatus;

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
