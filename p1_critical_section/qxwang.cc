/*
This test case prints a fibonacci sequence continuously by four cores and the core's name.
It tests whether the four cores can run by order, avoid race condition and perform simple math calculation. 
If they do not run one by one, the test case fails.
*/

#include "debug.h"
#include "config.h"
#include "smp.h"
#include "critical.h"

volatile bool done[MAX_PROCS];
volatile int counters[MAX_PROCS];
int x = 0, y = 1, temp=0; 

void kernelMain() {
    auto me = SMP::me();

    critical_begin();

    Debug::printf("*** ");
    // Debug::printf("%s ",SMP::names[me]);
    Debug::printf("Fibonacci sequence begins!\n");
    Debug::printf("*** ");
    for (unsigned i=0; i<10; i++){
        temp = y;
        y = x+y;
        x = temp;
        Debug::printf("%d, ",x);
        counters[me] = 5000000;
        while (-- counters[me] != 0);
    }

    Debug::printf("\n");

    critical_end();

    done[SMP::me()] = true;
    if (SMP::me() == 0) {
        for (unsigned i=0; i<kConfig.totalProcs; i++) {
            while (!done[i]);
        }
        Debug::shutdown();
    } else {
        while(true) asm volatile("hlt");
    }
}
