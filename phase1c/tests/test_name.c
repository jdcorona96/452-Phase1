#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static int flag = 0;

static int
Unblocks(void *arg)
{
    int sem = (int) arg;
    int rc;

    flag = 1;
    USLOSS_Console("V on semaphore.\n");
    rc = P1_V(sem);
    assert(rc == P1_SUCCESS);
    return 12;
}

static int
Blocks(void *arg) 
{
    int sem = (int) arg;
    int rc;
    int pid;

    rc = P1_Fork("Unblocks", Unblocks, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("P on semaphore.\n");
    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);
    assert(flag == 1);
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
    rc = P1_SemCreate("semName", 0, &sem);
    char name[50];
    rc = P1_SemName(0, &name[0]);
    assert(strcmp(name, "semName") == 0);
    USLOSS_Console("Name: %s \n", name);
    assert(rc == P1_SUCCESS);
    // Unblocks V's semaphore before Block P's
    rc = P1_Fork("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}