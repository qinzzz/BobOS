#include "critical.h"
#include "atomic.h"
#include "smp.h"

SpinLock s;
volatile int mycounters[MAX_PROCS];

void critical_begin() {
    auto me = SMP::me();
    if(mycounters[me]==0){
        s.lock();
        mycounters[me]++;
    }
    else {
        mycounters[me]++;
    }
}

void critical_end() {
    auto me = SMP::me();
    if (mycounters[me]){
        mycounters[me]--;
    }
    if (mycounters[me]==0){
        s.unlock();
    }

}