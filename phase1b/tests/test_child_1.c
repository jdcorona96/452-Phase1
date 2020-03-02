#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

// getchildstatus(): No Children, No children quit, Invalid Tag

static int
Child(void *arg) 
{
    int rc, cid, status;
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);

    // No children
    rc = P1GetChildStatus(0, &cid, &status);
    assert(rc == P1_NO_CHILDREN);
    
    P1_Quit(11);
    return 0;
}
static int
Output(void *arg) 
{
    int rc, cid, pid, status;
    char *msg = (char *) arg;
    
    USLOSS_Console("%s", msg);
    
    rc = P1_Fork("Child", Child, "Child!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    

   
    
    // No children quit
    rc = P1GetChildStatus(0, &cid, &status);
    assert(rc == P1_NO_QUIT);

    // Switches to child, and tests dispatch rotate functionality :)
    P1Dispatch(TRUE);

    // Invalid Tag
    rc = P1GetChildStatus(2, &cid, &status);
    assert(rc == P1_INVALID_TAG);
    
    rc = P1GetChildStatus(0, &cid, &status);
    assert(rc == P1_SUCCESS);
    P1_Quit(11);
    // should not get here
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
    
    
    P1_Quit(11);
    // P1_Quit should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}