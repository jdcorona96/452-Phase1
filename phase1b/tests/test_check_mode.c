#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

static int
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    P1_Quit(11);
    // should not return
    return 0;
}

void
startup(int argc, char **argv)
{
    int rc, pid, val;

    P1EnableInterrupts();
    val = USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE);
    assert(val == USLOSS_DEV_OK);
    USLOSS_Console("Current mode bit: %d\n", USLOSS_PsrGet() & 0x1);

    // P1ProcInit() will first do a kernal check and find that the process is in user mode
    // since we set the mode bit to 0. It then halts.
    P1ProcInit();

    // should not get here
    USLOSS_Console("startup\n");
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}