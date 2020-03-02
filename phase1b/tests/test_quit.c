#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

// quit(): First Process Quit While It Has Children

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
    int rc, pid;
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    rc = P1_Fork("Child", Child, "Child!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    
    //First process trying to quit while 2nd process still exists as a child
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