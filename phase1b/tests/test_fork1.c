#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdlib.h>

// for verbose use: #define DEBUG
// or gcc -D DEBUG ...

#ifdef DEBUG
# define print(...) USLOSS_Console(__VA_ARGS__)
#else
# define print(...) // USLOSS_Console(__VA_ARGS__)
#endif

static int
DoSomething(void *arg) {
    print("Hello From Child %d\n", arg);
    P1_Quit((int) arg);

    assert(FALSE); // should not return
}

static int
MakeChild(void *arg) 
{
    int res, pid, status = !0;
    
    print("Make Child\n");

    res = P1_Fork("child", DoSomething, (void*) 1, USLOSS_MIN_STACK, 5, 0, &pid);
    assert(res == P1_SUCCESS);

    print("Set State to Waiting\n");

    res = P1SetState(P1_GetPid(), P1_STATE_JOINING, -1);
    assert(res == P1_CHILD_QUIT);

    P1Dispatch(TRUE);

    print("No Longer Waiting, Check Our Info\n");

    P1_ProcInfo info = {};
    res = P1_GetProcInfo(P1_GetPid(), &info);
    assert(res == P1_SUCCESS);
    print("%d\n", info.state);
    assert(info.state == P1_STATE_RUNNING);
    
    print("Make Another Child\n");

    res = P1_Fork("child1", DoSomething, (void*) 2, USLOSS_MIN_STACK, 4, 0, &pid);
    assert(res == P1_SUCCESS);

    P1Dispatch(TRUE);

    res = P1GetChildStatus(0, &pid, &status);
    assert(res == P1_SUCCESS);
    assert(status == 1 || status == 2);
    assert(pid != P1_GetPid());

    print("Child %d Recycled\n", status);

    print("Wait for other Child\n");

    res = P1GetChildStatus(0, &pid, &status);
    assert(res == P1_SUCCESS);
    assert(status == 1 || status == 2);
    assert(pid != P1_GetPid());

    print("Child %d Recycled\n", status);

    P1_Quit(P1_SUCCESS);

    assert(FALSE); // should not return
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    rc = P1_Fork("Parent (Deamon)", MakeChild, NULL, USLOSS_MIN_STACK, 6, 0, &pid);
        
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}