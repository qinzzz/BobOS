#include "stdint.h"
#include "smp.h"
#include "debug.h"
#include "config.h"
#include "threads.h"

#define N 10000

/* Called by one CPU */
void kernelMain(void) {
    int nCpus = kConfig.totalProcs;
    int nThreads = nCpus * 2;
    volatile uint32_t *perCpu = new uint32_t[nCpus];
    volatile uint32_t *perThread = new uint32_t[nThreads];

    for (int i = 0; i<nCpus; i++) perCpu[i] = 0;

    for (int i = 0; i<nThreads; i++) perThread[i] = 0;

    Atomic<int> running { nThreads };

    for (int i = 0; i<nThreads; i++) {
        thread([i,perCpu,perThread,&running]() {
            for (int j = 0; j < N; j++) {
                perCpu[SMP::me()] ++;
                perThread[i] ++;
                //Debug::printf("thread %d did %d\n",i,perThread[i]);
                // if (perThread[i] == 10000){
                //     Debug::printf("thread %d end!!!!!!!\n",i);
                //     // sleep(100);
                // }
                yield();
            }
            running.fetch_add(-1);
        });
    }

    while (running.get() != 0) yield();
    for (int i=0; i<nCpus; i++) {
        Debug::printf("| cpu %d did %d\n",i,perCpu[i]);
        if (perCpu[i] == 0) {
            Debug::printf("*** cpu %d didn't do any work\n",i);
        }
    }
    for (int i=0; i<nThreads; i++) {
        Debug::printf("| thread %d did %d\n",i,perThread[i]);
        if (perThread[i] != N) {
            Debug::printf("*** thread %d did %d\n",i,perThread[i]);
        }
    }
    Debug::printf("*** done\n");
}

