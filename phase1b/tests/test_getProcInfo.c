#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

// getProcInfo(): Valid and invalid testing

static int
Output(void *arg) 
{
    int rc, pid;
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);

    // Invalid PID
    P1_ProcInfo info;
    rc = P1_GetProcInfo(1, &info);
    assert(rc == P1_INVALID_PID);

    // Valid PID
    pid = P1_GetPid();
    rc = P1_GetProcInfo(pid, &info);
    assert(rc == P1_SUCCESS);
    // This should print Hello
    USLOSS_Console("Name : %s\n", info.name);
    USLOSS_Console("sid : %d\n", info.sid);
    USLOSS_Console("priority : %d\n", info.priority);
    assert(info.priority == 1);
    USLOSS_Console("tag : %d\n", info.tag);
    assert(info.tag == 0);
    USLOSS_Console("parent : %d\n", info.parent);
    assert(info.parent == -1);
    USLOSS_Console("numChildren : %d\n", info.numChildren);
    assert(info.numChildren == 0);
    
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