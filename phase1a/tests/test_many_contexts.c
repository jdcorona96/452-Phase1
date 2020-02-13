#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>

/*
static void
Output2(void *arg) {
    USLOSS_Console(" This is second context\n");
    int *freeCid = (int*) arg;

    int rc = P1ContextFree(*freeCid);    
    assert(rc == P1_SUCCESS);
    rc = P1ContextSwitch(*freeCid);
    assert(rc == P1_INVALID_CID);
    USLOSS_Halt(0);
}


static void
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    int cid2;
    int cidArg = cid2;
    int rc = P1ContextCreate(Output2, &cidArg, USLOSS_MIN_STACK, &cid2);
    assert(rc == P1_SUCCESS);
    rc = P1ContextSwitch(cid2);
    
    
    // should not return  
    USLOSS_Console("this should not appear\n");
    assert(0);
//    USLOSS_Halt(0);
}

*/
void
startup(int argc, char **argv)
{
    int cid1;
    int rc;
    P1ContextInit();

    int i;
    for (i = 0; i < 55; i++) {
        rc = P1ContextCreate(NULL, NULL, USLOSS_MIN_STACK, &cid1);
        USLOSS_Console("current cid: %d\n",cid1);
        USLOSS_Console("current ite: %d\n\n", i);
        if (i < 50)
            assert(rc == P1_SUCCESS);
        else 
            assert(rc == P1_TOO_MANY_CONTEXTS);
    }
    USLOSS_Halt(0);
    /*
    rc = P1ContextCreate(Output, " Hello World!\n", USLOSS_MIN_STACK, &cid1);
    assert(rc == P1_SUCCESS);

    rc = P1ContextSwitch(cid1);
        //USLOSS_Halt(0);
    assert(0);
    */
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
