/*
* Authors: Luke Cernetic || Joseph Corona
* File: phase1a.c
* Purpose: Simulates an operating system by managing USLOSS contexts.
*/

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

/*
* Sets up the array of contexts by setting all of the occupied values to 0.
*/
void P1ContextInit(void)
{
    //checking if function was called in kernel mode
    int psr = USLOSS_PsrGet();
    if (!(psr & USLOSS_PSR_CURRENT_MODE)) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
        
    }

    // initialize contexts as not occupied
	for (i = 0; i < P1_MAXPROC; i++){
		contexts[i].occupied = 0;
	}

}

/*
* Creates a context and puts it in an open slot of the contexts array.
*/
int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {

	//checking if function was called in kernel mode
    int psr = USLOSS_PsrGet();
    if (!(psr & USLOSS_PSR_CURRENT_MODE)) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);    
    }

	//checking if stack size is too small
	if (stacksize < USLOSS_MIN_STACK){
		return P1_INVALID_STACK;
	}
	
	i = 0;
	while (contexts[i].occupied == 1) {
		i++;
		if (i > P1_MAXPROC-1){
			return P1_TOO_MANY_CONTEXTS;
		}
	 }
	*cid = i;
	contexts[i].occupied = 1;

	char *tempStack = malloc(sizeof(char) *stacksize);

	USLOSS_ContextInit(&contexts[i].context, tempStack, stacksize,
					   P3_AllocatePageTable(i), &launch);


	contexts[i].startFunc = func;
	contexts[i].startArg = arg;
    contexts[i].stack = &tempStack;

    return P1_SUCCESS;
}

/*
* Switches the current context to the context at the location given by the cid
* parameter.
*/
int P1ContextSwitch(int cid) {

	//checking if function was called in kernel mode
    int psr = USLOSS_PsrGet();
    if (!(psr & USLOSS_PSR_CURRENT_MODE)) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
        
    }

	//returns P1_INVALID_CID if cid is not occupied or is too big or too small.
	if (cid < 0 || cid > P1_MAXPROC - 1 || !contexts[cid].occupied){
		return P1_INVALID_CID;
	}

	//if the current cid is not valid, switch from null context.
    if (currentCid < 0 || currentCid > P1_MAXPROC - 1) {
        currentCid = cid;
        USLOSS_ContextSwitch(NULL, &contexts[cid].context);
    } 
	//switch to specified context
	else {
        int tempCur = currentCid;
        currentCid = cid;
        USLOSS_ContextSwitch(
            &contexts[tempCur].context, &contexts[cid].context);
    }
    return P1_SUCCESS;
}

/*
* Frees all allocated space (the stack) and sets the current context occupied to 0.
*/
int P1ContextFree(int cid) {
    
    int psr = USLOSS_PsrGet();
    if (!(psr & USLOSS_PSR_CURRENT_MODE)) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
        
    }
    
    // free the stack and mark the context as unused
    if (cid < 0 || cid > P1_MAXPROC-1 || !contexts[cid].occupied)
        return P1_INVALID_CID;
    else if (cid == currentCid)
        return P1_CONTEXT_IN_USE;
    else {
        free(*contexts[cid].stack);
        contexts[cid].occupied = 0;
        P3_FreePageTable(cid);

        return P1_SUCCESS;
    }
}


void 
P1EnableInterrupts(void) {

    int psr = USLOSS_PsrGet();
    if (!(psr & USLOSS_PSR_CURRENT_MODE)) {
        USLOSS_IllegalInstruction();
        USLOSS_Halt(0);
        
    }
    
    // set the interrupt bit in the PSR
    int res = USLOSS_PsrSet(psr | USLOSS_PSR_CURRENT_INT);
    assert(res == USLOSS_DEV_OK);
}

int 
P1DisableInterrupts(void) {
      
    int psr = USLOSS_PsrGet();
    if (!(psr & USLOSS_PSR_CURRENT_MODE)) {
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


