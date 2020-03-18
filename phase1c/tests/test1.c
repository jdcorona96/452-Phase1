#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>

void
startup(int argc, char **argv)
{
    int rc;
    int sem;
	
	
	rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);

	rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_DUPLICATE_NAME);
	USLOSS_Console("Duplicate name\n");
	

	rc = P1_SemCreate("\0", 0, &sem);
    assert(rc == P1_NAME_IS_NULL);
 	USLOSS_Console("Name is null\n");

	rc = P1_SemCreate("123123123131231231231231123131313131231231231231231231231231231313311231231313131313123123123131313121331231231231313131332123132131213212123123123123123132123312132131313131312312312313", 0, &sem);
    assert(rc == P1_NAME_TOO_LONG);
 	USLOSS_Console("Name is too long\n");

	int i;
	int curr = 0;
	char str[5];

	for (i = 1; i < P1_MAXSEM; i++) {
		sprintf(str, "%d", curr);
		rc = P1_SemCreate(str, 0, &sem);
		assert(rc == P1_SUCCESS);
		curr++;
	}
		
	rc = P1_SemCreate("2001", 0, &sem);
	assert(rc == P1_TOO_MANY_SEMS);
	USLOSS_Console("Too many sems\n");
	USLOSS_Console("Completed. SIMULATOR_TRAP expected.\n\n\n");

}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
