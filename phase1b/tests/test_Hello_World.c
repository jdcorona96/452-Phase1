#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

static int
World(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    P1_Quit(11);
    // should not return
    return 0;
}
static int
Hello(void *arg) 
{
    int rc, cid, pid, status;
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    rc = P1_Fork("World", World, "World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    P1Dispatch(TRUE);

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
    rc = P1_Fork("Hello", Hello, "Hello ", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}