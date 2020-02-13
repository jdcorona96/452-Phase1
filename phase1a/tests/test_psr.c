#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

static void
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    int psr = USLOSS_PsrGet();
    USLOSS_Console("psr: %d\n",psr);
    if (P1DisableInterrupts()) {
        USLOSS_Console("psr was enabled\n");
        assert(psr & (1<<1));
    } else {
        USLOSS_Console("psr was disabled\n");
        assert(!(psr & (1<<1)));
    }

    P1EnableInterrupts();
    psr = USLOSS_PsrGet();
    USLOSS_Console("psr: %d\n", psr);
    if (P1DisableInterrupts()) {
        USLOSS_Console("psr was enabled\n");
        assert(psr & (1<<1));
    } else {
        USLOSS_Console("psr was disabled\n");
        assert(!(psr & (1<<1)));
    }

    //P1_EnableInterrupts();
    //assert(psr != USLOSS_PsrGet());
    //assert(USLOSS_PsrGet() 

    USLOSS_Console("everything seems good\n");
    USLOSS_Halt(0);
}

void
startup(int argc, char **argv)
{
    int cid;
    int rc;
    P1ContextInit();
    rc = P1ContextCreate(Output, "next context\n", USLOSS_MIN_STACK, &cid);
    assert(rc == P1_SUCCESS);
    rc = P1ContextSwitch(cid);
    // should not return
    assert(rc == P1_SUCCESS);
    USLOSS_Console("this should not appear\n");
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
