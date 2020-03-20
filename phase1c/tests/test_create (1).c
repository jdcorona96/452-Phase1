#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static int flag = 0;

void dummy(int type, void *arg);

static int
Unblocks(void *arg)
{
    int sem = (int) arg;
    int rc;

    flag = 1;
    USLOSS_Console("V on semaphore.\n");
    
    // Invalid V Semaphore ID
    rc = P1_V(-1);
    assert(rc == P1_INVALID_SID);

    rc = P1_V(sem);
    assert(rc == P1_SUCCESS);
    // will never get here as Blocks will run and call USLOSS_Halt.
    return 12;
}

static int
Blocks(void *arg) 
{
    int sem = (int) arg;
    int rc;
    int pid;

    rc = P1_Fork("Unblocks", Unblocks, (void *) sem, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("P on semaphore.\n");
    
    // Invalid P Semaphore ID
    rc = P1_P(-1);
    assert(rc == P1_INVALID_SID);

    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);
    assert(flag == 1);

    // Free Then Invalid Semaphore call
    rc = P1_SemFree(sem);
    assert(rc == P1_SUCCESS);
    rc = P1_P(sem);
    assert(rc == P1_INVALID_SID);
    
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
    int semReal, sem;

    USLOSS_IntVec[USLOSS_CLOCK_INT] = dummy;

    P1SemInit();
    rc = P1_SemCreate("sem", 0, &semReal);
    assert(rc == P1_SUCCESS);

    // Duplicate Name
    rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_DUPLICATE_NAME);

    // NULL Name
    rc = P1_SemCreate(NULL, 0, &sem);
    assert(rc == P1_NAME_IS_NULL);

    // Name Too Long
    rc = P1_SemCreate("01234567890123456789012345678901234567890123456789012345678901234567890123456789 ", 0, &sem);
    assert(rc == P1_NAME_TOO_LONG);

    //Too many Semaphores   
    int i = 0;
    for (i = 0; i < P1_MAXSEM-1; i++) {
        char v [5];

        int digit = i / 1000;
        v[0] = (digit + '0');

        digit = (i / 100) % 10;
        v[1] = (digit + '0');

        digit = (i / 10) % 10;
        v[2] = (digit + '0');

        digit = i % 10;
        v[3] = (digit + '0');

        v[4] = '\0';
        USLOSS_Console("v : %s\n", v);
        rc = P1_SemCreate(v, 0, &sem);
        assert(rc == P1_SUCCESS);
    }
    rc = P1_SemCreate("Too Many Of Us", 0, &sem);
    assert(rc == P1_TOO_MANY_SEMS);

    // Blocks blocks then Unblocks unblocks it
    rc = P1_Fork("Blocks", Blocks, (void *) semReal, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
