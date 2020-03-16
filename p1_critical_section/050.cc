#include "debug.h"
#include "config.h"
#include "smp.h"
#include "critical.h"

volatile bool done[MAX_PROCS];
volatile bool alldone[2];
volatile int counter;

/*
- Bugs/Edge Cases
    - This test tests for multiple bugs.
    - The first is the ability to handle
    nested critical sections
    - Another is the ability to handle
    sequential critical sections
    - One more is if only one core accesses
    "counter" at any given time, which given
    that only one core should be in a critical
    section at once should be a valid requirement.
    - A final one is being able to output different
    amounts of numbers with different counting
    intervals inbetween.
- Expectations/Assertions
    - Two cores shouldn't enter the space between
    critical_begin and critical_end at the same time
    - At least one core will have a "me" id value
    of 0.
    - Cores will execute their code in a linear
    fashion (critical_begin will run before
    critical_end, etc.)
    - Cores will periodically switch off of being
    "active", even while busy-waiting.
    - Assuming four cores only to match .ok file
*/

void kernelMain() {
    auto me = SMP::me();



    critical_begin();

    critical_begin();

    Debug::printf("*");
    Debug::printf("*");
    Debug::printf("*");
    Debug::printf(" ");
    for (unsigned i=0; i<10; i++) {
        Debug::printf("%d",i);
        counter = 10000;
        while (-- counter != 0);
    }
    Debug::printf("\n");

    critical_end();

    critical_end();



    done[me] = true;
    if (me == 0) {
        for (unsigned i=0; i<kConfig.totalProcs; i++) {
            while (!done[i]);
            done[i] = false;
        }
        Debug::printf("*****\n");
        alldone[0] = true;
    } else {
        while(!alldone[0]);
    }



    critical_begin();

    Debug::printf("*");
    Debug::printf("*");
    Debug::printf("*");
    Debug::printf(" ");
    for (unsigned i=0; i<3; i++) {
        Debug::printf("%d",6+i);
        counter = 6431247;
        while (-- counter != 0);
    }
    Debug::printf("\n");

    critical_end();



    done[me] = true;
    if (me == 0) {
        for (unsigned i=0; i<kConfig.totalProcs; i++) {
            while (!done[i]);
            done[i] = false;
        }
        Debug::printf("*****\n");
        alldone[1] = true;
    } else {
        while(!alldone[1]);
    }



    critical_begin();

    Debug::printf("*");
    Debug::printf("*");
    Debug::printf("*");
    Debug::printf(" ");
    for (unsigned i=2; i<8; i++) {
        Debug::printf("%d",9-i);
        counter = 261;
        while (-- counter != 0);
    }
    Debug::printf("\n");

    critical_end();



    done[me] = true;
    if (me == 0) {
        for (unsigned i=0; i<kConfig.totalProcs; i++) {
            while (!done[i]);
        }
        Debug::printf("*****\n");
        Debug::shutdown();
    } else {
        while(true) asm volatile("hlt");
    }
}
