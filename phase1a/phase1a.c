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
	int 			cid;
    // you'll need more stuff here
} Context;

static Context   contexts[P1_MAXPROC];
int i = 0;
static int currentCid = -1;

//TEMP USLOSS_Context STRUCT and stack
//USLOSS_Context *tempContext;



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
    // initialize contexts
	//MIGHT HAVE ISSUE BECAUSE MALLOC IS VOID POINTER
	for (i = 0; i < P1_MAXPROC; i++){
		contexts[i].cid = -1;
	}

}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    int result = P1_SUCCESS;

	if (stacksize < USLOSS_MIN_STACK){
		return P1_INVALID_STACK;
	}
	
	char tempStack[stacksize];



	while (contexts[i].cid == -1) {
		*cid = i;
		i++;
		if (i > P1_MAXPROC){
			result = P1_TOO_MANY_CONTEXTS;
			break;
		}

	}

	//tempContext = (USLOSS_Context*) malloc(sizeof(USLOSS_Context));
	USLOSS_Context *tempContext = (USLOSS_Context*) malloc(sizeof(USLOSS_Context));

	USLOSS_ContextInit(tempContext, tempStack, stacksize,
						P3_AllocatePageTable(i), &launch);


	Context current;

	current.startFunc = func;
	current.startArg = arg;
	current.context = *tempContext;



	contexts[i] = current;

	
	
	
	// find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    return result;
}

int P1ContextSwitch(int cid) {
    int result = P1_SUCCESS;
	if (cid < 0 || cid > P1_MAXPROC - 1){
		return P1_INVALID_CID;
	}

	USLOSS_ContextSwitch(&contexts[currentCid].context, &contexts[cid].context);

    // switch to the specified context
    return result;
}

int P1ContextFree(int cid) {
    int result = P1_SUCCESS;
    // free the stack and mark the context as unused
    return result;
}


void 
P1EnableInterrupts(void) 
{
    // set the interrupt bit in the PSR
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int 
P1DisableInterrupts(void) 
{
    int enabled = FALSE;
    // set enabled to TRUE if interrupts are already enabled
    // clear the interrupt bit in the PSR
    return enabled;
}
