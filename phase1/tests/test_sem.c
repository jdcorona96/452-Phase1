/*
  Test case for basic Semaphores. Ensures that P1_P blocks, then child executes and prints 0, then child unblocks parent and parent prints 1

  should print :

  ---------Starting Test SEM ----------
 0
 1
 ---------Ending Test SEM ----------

*/

#include "phase1.h"
#include <stdio.h>

P1_Semaphore sem1;

int child(void *arg) {
    USLOSS_Console(" %d \n", 0);
    P1_V(sem1);
    return 0;
}

int P2_Startup(void *arg){

    USLOSS_Console(" \n---------Starting Test SEM ----------\n");
    P1_Fork("child", child, NULL, USLOSS_MIN_STACK, 4 , 0);
    int result = P1_SemCreate("semaphore" , 0, &sem1 );
    if ( result != 0 )
    {
        USLOSS_Console("Could not create semaphore, result: %d\n", result );
    }
    
    P1_P(sem1);
    USLOSS_Console(" %d \n", 1);

    USLOSS_Console(" ---------Ending Test SEM ----------\n");
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
