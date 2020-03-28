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
    int prevint = p1disableinterrupts();

    // check kernel mode
    kernelmode();

    // cehking if type is valid
    if (type < 0 || type > 3)
        return p1_invalid_type;
    
    // checking if unit is valid based on type
    if (type == 0 || type == 1) {
        if (unit != 0)
            return p1_invalid_unit;
    }
    if (type == 2) {
        if (unit != 0 && unit != 1)
            return p1_invalid_unit;
    }
    if (type == 3) {
        if (unit < 0 || unit > 3)
            return p1_invalid_unit;
    }

    // p device's semaphore
    p1_p(devsem[type][unit]);

    // set *status to device's status
    // todo: how to get status??
    // suggestion: set up global variables to store more info on 
    //             each interrupt handler
    
    // restore interrupts
    if (prevint)
        p1enableinterrupts();

    return p1_success;
}


int 
p1_wakeupdevice(int type, int unit, int status, int abort) 
{

    // disable interrupts
    int prevint = p1disableinterrupts();

    // check kernel mode
    kernelmode();

    // cehking if type is valid
    if (type < 0 || type > 3)
        return p1_invalid_type;
    
    // checking if unit is valid based on type
    if (type == 0 || type == 1) {
        if (unit != 0)
            return p1_invalid_unit;
    }
    if (type == 2) {
        if (unit != 0 && unit != 1)
            return p1_invalid_unit;
    }
    if (type == 3) {
        if (unit < 0 || unit > 3)
            return p1_invalid_unit;
    }

    // save device's status to be used by p1_waitdevice
    // save abort to be used by p1_waitdevice

    // v device's semaphore
    p1_v(devsem[type][unit]);

    // restore interrupts
    if (prevint)
        p1enableinterrupts();

    return p1_success;

}

static void
devicehandler(int type, void *arg) 
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
    rc = p1_fork("p2_startup", p2_startup, null, 4 * usloss_min_stack, 2 , 0, &pid);
    assert(rc == p1_success);

    // enable interrupts
    p1enableinterrupts();

	rc = p1_getprocinfo(pid, info);
	assert(rc == p1_success);
	while (info->numchildren != 0){

		
		rc = p1getchildstatus(0, pid, status);
		
		// if a child exists
		if (rc == p1_no_quit){
			usloss_waitint();
		}
		else {
			//check other tag
			rc = p1getchildstatus(1, pid, status);

			//if a child exists
			if (rc == p1_no_quit) {
				usloss_waitint();
			}
		}

		rc = p1_getprocinfo(pid, info);
		assert(rc == p1_success);
	}


    // while sentinel has children
    // phase1b: procinfo - get number of children

    //      get children that have quit via p1getchildstatus (either tag)
    //      wait for an interrupt via usloss_waitint
	

	// no children left
    usloss_console("sentinel quitting.\n");
    return 0;
} /* end of sentinel */

int 
p1_join(int tag, int *pid, int *status) 
{
	// disable interrupts
    int prevint = p1disableinterrupts();

    // check kernel mode
    kernelmode();



	int pid = p1_getpid();
	p1_procinfo info;

	rc = p1_getprocinfo(pid, info);
	assert(rc == p1_success);

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
