#include "debug.h"
#include "config.h"
#include "smp.h"
#include "critical.h"

volatile bool done[MAX_PROCS];
volatile int counters[MAX_PROCS];

void kernelMain() {
    auto me = SMP::me();

    critical_begin();

    critical_begin();

    Debug::printf("*");
    Debug::printf("*");
    Debug::printf("*");
    Debug::printf(" ");
    for (unsigned i=0; i<20; i++) {
        Debug::printf("%d",i);
        counters[me] = 1000000;
        while (-- counters[me] != 0);
    }
    Debug::printf("\n");

    critical_end();

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
