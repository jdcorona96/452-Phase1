#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

static int flag = 0;

static int
func3(void *arg)
{
	int sem = (int) arg;
    int rc;
	
	USLOSS_Console("func3\n");
	rc = P1_P(sem);
	assert(rc == P1_SUCCESS);
	return 0;
}

static int
func2(void *arg)
{
    int sem = (int) arg;
    int rc;

    flag = 1;
    USLOSS_Console("V on semaphore.\n");
    rc = P1_V(sem);
    assert(rc == P1_SUCCESS);
    // will never get here as Blocks will run and call USLOSS_Halt.
    return 12;
}

static int
func1(void *arg) 
{
    int sem = (int) arg;
    int rc;
    int pid;

    rc = P1_Fork("func2", func2, (void *) sem, USLOSS_MIN_STACK, 3, 0, &pid);
	rc = P1_Fork("func3", func3, (void *) sem, USLOSS_MIN_STACK, 2, 0, &pid);
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
    rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);
    // Blocks blocks then Unblocks unblocks it
    rc = P1_Fork("func1", func1, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}