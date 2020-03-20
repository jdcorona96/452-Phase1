#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>



static int
UnblocksZero(void *arg)
{
    int sem = (int) arg;
    int rc;

    USLOSS_Console("2: P on sem UnblockZero\n");
    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("7: V on sem UnblockZero.\n");
    rc = P1_V(sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("8: Quit UnblockZero \n");
    P1_Quit(11);
    USLOSS_Console("Still running 0 \n");
    assert(0);
    
    // Should not return.
    return 12;
}

static int
UnblocksOne(void *arg)
{
    int sem = (int) arg;
    int rc;

    USLOSS_Console("3: P on sem UnblockOne\n");
    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("9: V on semaphore UnblockOne.\n");
    rc = P1_V(sem);
    USLOSS_Console("Still running 1 \n");
    assert(0);

    // Should not return.
    return 12;
}

static int
UnblocksTwo(void *arg)
{
    int sem = (int) arg;
    int rc;

    USLOSS_Console("4: V on semaphore UnblocksTwo A.\n");
    rc = P1_V(sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("6: V on sem UnblocksTwo B. \n");
    rc = P1_V(sem);
    assert(rc == P1_SUCCESS);
    assert(0);
    // Should not return.
    return 12;
}

static int
Blocks(void *arg) 
{
    int sem = (int) arg;
    int rc;
    int pid0;
    int pid1;
    int pid2;
    rc = P1_Fork("Unblocks0", UnblocksZero, (void *) sem, USLOSS_MIN_STACK, 2, 0, &pid0);
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Unblocks1", UnblocksOne, (void *) sem, USLOSS_MIN_STACK, 3, 0, &pid1);
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Unblocks2", UnblocksTwo, (void *) sem, USLOSS_MIN_STACK, 4, 0, &pid2);
    USLOSS_Console("1: P on semaphore Blocks A.\n");
    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);;
    USLOSS_Console("5: P on sem Blocks B\n");
    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);;
    USLOSS_Console("\nBack to Blocks\n");
    rc = P1_SemFree(0);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Sem Free Passed\n\n");
    USLOSS_Console("Test passed.\n");
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
    int sem;

    P1SemInit();
    rc = P1_SemCreate("semZero", 0, &sem);
    assert(rc == P1_SUCCESS);
    
    rc = P1_Fork("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}