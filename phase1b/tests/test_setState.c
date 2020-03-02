#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdlib.h>

// setState(): Invalid PID, Invalid state, Child Quit

static int
Child(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    P1_Quit(11);
    // should not return
    return 0;
}
static int
Output(void *arg) 
{
    int rc, cid, pid, status;
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    
    rc = P1_Fork("Child", Child, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    P1Dispatch(TRUE);

    // Invalid PID
    rc = P1SetState(100, P1_STATE_QUIT, 0);
    assert(rc == P1_INVALID_PID);

    // Invalid State
    rc = P1SetState(pid, 20, 0);
    assert(rc == P1_INVALID_STATE);

    // Test Child_quit error code in setState
    cid = P1_GetPid();
    rc = P1SetState(cid, P1_STATE_JOINING, 0);
    assert(rc == P1_CHILD_QUIT);

    rc = P1GetChildStatus(0, &cid, &status);
    assert(rc == P1_SUCCESS);
    P1_Quit(11);
    // should not return
    return 0;
}
void
startup(int argc, char **argv)
{
    int rc, pid;
    P1ProcInit();
    USLOSS_Console("startup\n");
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}