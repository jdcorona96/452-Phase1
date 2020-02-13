#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

extern  USLOSS_PTE  *P3_AllocatePageTable(int cid);
extern  void        P3_FreePageTable(int cid);

typedef struct Context {
    void            (*startFunc)(void *);
    void            *startArg;
    USLOSS_Context  context;
	int 			occupied;
	char			**stack;
    // you'll need more stuff here
} Context;

static Context   contexts[P1_MAXPROC];
int i = 0;
static int currentCid = -1;


/*
 * Helper function to call func passed to P1ContextCreate with its arg.
 */
static void launch(void)
{
    assert(contexts[currentCid].startFunc != NULL);
    contexts[currentCid].startFunc(contexts[currentCid].startArg);
}

void P1ContextInit(void)
{
    //checking if function was called in kernel mode
    int psr = USLOSS_PsrGet();
    if (psr ^ USLOSS_PSR_CURRENT_MODE) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
        
    }

    // initialize contexts

	for (i = 0; i < P1_MAXPROC; i++){
		contexts[i].occupied = 0;
	}

}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    int result = P1_SUCCESS;

    int psr = USLOSS_PsrGet();
    if (psr ^ USLOSS_PSR_CURRENT_MODE) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);    
    }

	if (stacksize < USLOSS_MIN_STACK){
		return P1_INVALID_STACK;
	}
	
	i = 0;
	while (contexts[i].occupied == 1) {
		i++;
		if (i > P1_MAXPROC-1){
			result = P1_TOO_MANY_CONTEXTS;
			return result;
		}
	 }
	USLOSS_Console("%d", i);
	*cid = i;
	contexts[i].occupied = 1;

	char *tempStack = malloc(sizeof(char) *stacksize);

	USLOSS_Console("%d", i);
	USLOSS_ContextInit(&contexts[i].context, tempStack, stacksize,
					   P3_AllocatePageTable(i), &launch);


	contexts[i].startFunc = func;
	contexts[i].startArg = arg;
    contexts[i].stack = &tempStack;
	//currentCid = *cid;

	
	// find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    return result;
}

int P1ContextSwitch(int cid) {
	USLOSS_Console("4");

    int psr = USLOSS_PsrGet();
    if (psr ^ USLOSS_PSR_CURRENT_MODE) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
        
    }

    int result = P1_SUCCESS;
	if (cid < 0 || cid > P1_MAXPROC - 1 || !contexts[cid].occupied){
		return P1_INVALID_CID;
	}

    if (currentCid < 0 || currentCid > P1_MAXPROC-1) {
        currentCid = cid;
        USLOSS_ContextSwitch(NULL, &contexts[cid].context);
    } else {
        int tempCur = currentCid;
        currentCid = cid;
        USLOSS_ContextSwitch(
            &contexts[tempCur].context, &contexts[cid].context);
    }
    // switch to the specified context
    return result;
}

int P1ContextFree(int cid) {
	USLOSS_Console("5");
    
    int psr = USLOSS_PsrGet();
    if (psr ^ USLOSS_PSR_CURRENT_MODE) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
        
    }
    
    int result = P1_SUCCESS;
    // free the stack and mark the context as unused
    if (cid < 0 || cid > P1_MAXPROC-1 || !contexts[cid].occupied)
        return P1_INVALID_CID;
    else if (cid == currentCid)
        return P1_CONTEXT_IN_USE;
    else {
        free(*contexts[cid].stack);
        contexts[cid].occupied = 0;
        P3_FreePageTable(cid);

        return result;
    }
}


void 
P1EnableInterrupts(void) 
{
	USLOSS_Console("6");
    
    int psr = USLOSS_PsrGet();
    if (psr ^ USLOSS_PSR_CURRENT_MODE) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
        
    }
    
    // set the interrupt bit in the PSR
    int res = USLOSS_PsrSet(psr | USLOSS_PSR_CURRENT_INT);
    assert(res == USLOSS_DEV_OK);
}



/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int 
P1DisableInterrupts(void) 
{
	USLOSS_Console("7");
    
    
    int psr = USLOSS_PsrGet();
    if (psr ^ USLOSS_PSR_CURRENT_MODE) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
        
    }
    
    int enabled = FALSE;
    // set enabled to TRUE if interrupts are already enabled
    // clear the interrupt bit in the PSR
    int res = USLOSS_PsrSet(psr & 0xd);
    assert(res == USLOSS_DEV_OK);
    if (psr & USLOSS_PSR_CURRENT_INT)
        enabled = TRUE;
    
    return enabled;
}


