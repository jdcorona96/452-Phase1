#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdlib.h>

static int curr = 65;
static int counter = 1;
static int Other(void *arg);
static int Output(void *arg);

static int
Output(void *arg)
{
    int pid, rc;
    char *msg = (char *) arg;
    curr++;
    char v [2];
    v[0] = (char) curr;
    v[1] = '\0';
    rc = P1_Fork(v, Other, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    if (rc == P1_TOO_MANY_PROCESSES) {
        P1_ProcInfo info = {};
        rc = P1_GetProcInfo(P1_GetPid(), &info);
        assert(rc == P1_SUCCESS && info.numChildren == 49);
    }
    USLOSS_Console("%d: %s", counter, msg);
    counter++;
    P1_Quit(11);
    // should not return
    return 0;
}

static int
Other(void *arg) {
    int pid, rc;
    char *msg = (char *) arg;
    curr++;
    char v [2];
    v[0] = (char) curr;
    v[1] = '\0';
    rc = P1_Fork(v, Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    USLOSS_Console("%d: %s", counter, msg);
    counter++;
    P1_Quit(11);
    // should not return
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid, rc;
    P1ProcInit();
    USLOSS_Console("startup\n");
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}