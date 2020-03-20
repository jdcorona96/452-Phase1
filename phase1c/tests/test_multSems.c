#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

static int flag = 0;

static int
Unblocks(void *arg)
{
    int semZero = (int) arg;
    int rc;

    flag = 1;
    USLOSS_Console("2: V on semaphore 0.\n");
    rc = P1_V(semZero);
    assert(rc == P1_SUCCESS);
    // From here we just use sid's directly because we know
    // them. There could be fancy way of making the global
    // but we just know sems of sid's 0,1,2.
    USLOSS_Console("4: v on semaphore 1.\n");
    rc = P1_V(1);
    // Should goto BLOCKs.
    assert(rc == P1_SUCCESS);
    assert(flag == 2);
    USLOSS_Console("6: V on semaphore 2.\n");
    flag = 3;
    rc = P1_V(2);
 
    assert(rc == P1_SUCCESS);
    P1_Quit(11);
   
    assert(0);
    // will never get here as Blocks will run and call USLOSS_Halt.
    return 12;
}

static int
Blocks(void *arg) 
{
    int semZero = (int) arg;
    int rc;
    int pid;

    rc = P1_Fork("Unblocks", Unblocks, (void *) semZero, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("1: P on semaphore 0.\n");
    rc = P1_P(semZero);
    assert(rc == P1_SUCCESS);
    assert(flag == 1);
    flag = 2;
    USLOSS_Console("3: P on semaphore 1.\n");
    rc = P1_P(1);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("5: P on semaphore 2.\n");
    rc = P1_P(2);
    //should swap back
    assert(rc == P1_SUCCESS);
    assert(flag == 3);
    USLOSS_Console("7: Test passed.\n");
    USLOSS_Halt(0);
    // should not return
    assert(0);
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    int semZero;
    int semOne;
    int semTwo;
    P1SemInit();
    rc = P1_SemCreate("sem0", 0, &semZero);
    assert(rc == P1_SUCCESS);
    rc = P1_SemCreate("sem1", 0, &semOne);
    USLOSS_Console("sem1 : %d\n", semOne);
    assert(rc == P1_SUCCESS);
    rc = P1_SemCreate("sem2", 0, &semTwo);
    USLOSS_Console("sem2 : %d\n", semTwo);
    assert(rc == P1_SUCCESS);
    // Blocks blocks then Unblocks unblocks it
    rc = P1_Fork("Blocks", Blocks, (void *) semZero, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}