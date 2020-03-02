#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdlib.h>

// fork(): NULL name, Name Too Long, Duplicate Name

static int
Output(void *arg) 
{
    int rc, pid;
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    
    //Duplicate Name
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_DUPLICATE_NAME);

    P1_Quit(11);
    return 0;
}

void
startup(int argc, char **argv)
{
    int rc, pid;
    P1ProcInit();
    USLOSS_Console("startup\n");

    //NULL Name
    rc = P1_Fork(NULL, Output, "This shouldn't be possible...\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_NAME_IS_NULL);

    //Name Too Long
    rc = P1_Fork("01234567890123456789012345678901234567890123456789012345678901234567890123456789 ", Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_NAME_TOO_LONG);

    //Duplicate Name
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    
    // P1_Quit should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}