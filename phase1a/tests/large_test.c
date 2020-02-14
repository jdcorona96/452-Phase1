/**
 * Modified test case for phase1a
 * Output should be the following:
        Hello from context 1!
        Hello from context 2!

        You should see a simulator trap:
        SIMULATOR TRAP: USLOSS_IntVec[USLOSS_ILLEGAL_INT] is NULL!

        Aborted (core dumped)
 */ 

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

int cid[P1_MAXPROC+1];

//Second Context
static void
Output2(void *arg)
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);

   
    USLOSS_Console("\nYou should see a simulator trap:\n"); 

    //PSR is set to '0010'. This is an illegal value.
    assert(USLOSS_PsrSet(2) != USLOSS_ERR_INVALID_PSR);

    P1ContextInit();

    USLOSS_Console("You shouldn't see this message. Your function executed in user mode instead of kernel mode.\n");

    USLOSS_Halt(0);
}

//First context.
static void
Output(void *arg) 
{
    char *msg = (char *) arg;

    int rc;

    USLOSS_Console("%s", msg);

    rc = P1ContextSwitch(cid[1]);

    assert(0);
}

void
startup(int argc, char **argv)
{
    int rc[P1_MAXPROC+1];

    P1ContextInit();

    //Create P1_MAXPROC# contexts
    int i;
    for (i = 0; i < P1_MAXPROC; i++)
    {
        rc[i] = P1ContextCreate(Output, "Hello World!\n", USLOSS_MIN_STACK, &cid[i]);
        assert(rc[i] != P1_TOO_MANY_CONTEXTS);
        assert(rc[i] == P1_SUCCESS);
    }

    //Check too many contexts
    rc[i+1] = P1ContextCreate(Output2, "Hello World!\n", USLOSS_MIN_STACK, &cid[i+1]);

    assert(rc[i+1] == P1_TOO_MANY_CONTEXTS);
    assert(rc[i+1] != P1_SUCCESS);

    //Free all the contexts
    for (i = 0; i < P1_MAXPROC; i++)
    {
        rc[i] = P1ContextFree(cid[i]);
        assert(rc[i] == P1_SUCCESS);
    }

    //Create 2 contexts
    rc[0] = P1ContextCreate(Output, "Hello from context 1!\n", USLOSS_MIN_STACK, &cid[0]);
    rc[1] = P1ContextCreate(Output2, "Hello from context 2!\n", USLOSS_MIN_STACK, &cid[1]);
    rc[2] = P1ContextCreate(Output, "Hello, I don't have a valid stack size!\n", USLOSS_MIN_STACK-1, &cid[2]);

    //Check min stack size
    assert(rc[2] == P1_INVALID_STACK);

    int val = USLOSS_PsrGet();

    //If this fails, something went terribly wrong
    assert(USLOSS_PsrSet(1) != USLOSS_ERR_INVALID_PSR);
    //Enable interrupts, make sure 2nd bit is now 1
    P1EnableInterrupts();
    assert(USLOSS_PsrGet() == 3);

    //Disable interupts, check return val, 2nd bit is 0
    int ret_val = P1DisableInterrupts();
    assert(ret_val == TRUE);
    assert(USLOSS_PsrGet() == 1);

    //Disable again, check return val is FALSE
    ret_val = P1DisableInterrupts();
    assert(ret_val == FALSE);

    //Check to make sure the psr doesn't change
    //In any bits but the 2nd
    assert(USLOSS_PsrSet(13) != USLOSS_ERR_INVALID_PSR);
    P1EnableInterrupts();

    //Check to make sure the 2nd bit is 1, other bits are unchanged
    assert(USLOSS_PsrGet() == 15);

    //Check return val again
    ret_val = P1DisableInterrupts();
    assert(ret_val == TRUE);

    //Check to make sure the 2nd bit is 0 again
    assert(USLOSS_PsrGet() == 13);

    //Restore psr
    assert(USLOSS_PsrSet(val) != USLOSS_ERR_INVALID_PSR);

    //Test cid > P1_MAXPROC
    ret_val = P1ContextSwitch(891375);
    assert(ret_val == P1_INVALID_CID);

    //Test switching to free context
    ret_val = P1ContextSwitch(10);
    assert(ret_val == P1_INVALID_CID);

    //Test invalid cid
    ret_val = P1ContextSwitch(-23);
    assert(ret_val == P1_INVALID_CID);

    //Switch to 1st context.
    rc[0] = P1ContextSwitch(cid[0]);
    // should not return
    assert(rc[0] == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}