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
    // initialize contexts

	for (i = 0; i < P1_MAXPROC; i++){
		contexts[i].occupied = 0;
	}

}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    int result = P1_SUCCESS;

	if (stacksize < USLOSS_MIN_STACK){
		return P1_INVALID_STACK;
	}
	
	i = 0;
	while (contexts[i].occupied == 1) {
		i++;
		if (i > P1_MAXPROC){
			result = P1_TOO_MANY_CONTEXTS;
			break;
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
	currentCid = *cid;

	
	// find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    return result;
}

int P1ContextSwitch(int cid) {
	USLOSS_Console("4");
    int result = P1_SUCCESS;
	if (cid < 0 || cid > P1_MAXPROC - 1){
		return P1_INVALID_CID;
	}

	USLOSS_ContextSwitch(&contexts[currentCid].context, &contexts[cid].context);

    // switch to the specified context
    return result;
}

int P1ContextFree(int cid) {
	USLOSS_Console("5");
    int result = P1_SUCCESS;
    // free the stack and mark the context as unused
    return result;
}


void 
P1EnableInterrupts(void) 
{
	USLOSS_Console("6");
    // set the interrupt bit in the PSR
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int 
P1DisableInterrupts(void) 
{
	USLOSS_Console("7");
    int enabled = FALSE;
    // set enabled to TRUE if interrupts are already enabled
    // clear the interrupt bit in the PSR
    return enabled;
}
