#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

// fork(): Double 6 Priority Processes, Invalid Tag, Stack Too Small

static int
Output(void *arg) 
{
    int rc, pid;
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    
    // Double 6 Priority Processes
    rc = P1_Fork("Hello2", Output, "Hello World?\n", USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_INVALID_PRIORITY);

    P1_Quit(11);
    // should not return
    return 0;
}

void
startup(int argc, char **argv)
{
    int rc, pid;
    P1ProcInit();
    
    //Invalid Tag
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 2, &pid);
    assert(rc == P1_INVALID_TAG);

    // Stack Too Small
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK - 1, 1, 0, &pid);
    assert(rc == P1_INVALID_STACK);
    
    USLOSS_Console("startup\n");
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);

    // P1_Fork() should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}