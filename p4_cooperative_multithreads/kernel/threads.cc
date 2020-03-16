#include "debug.h"
#include "smp.h"
#include "threads.h"
#include "atomic.h"
# include "future.h"


Queue<TCB> readyQ;
Queue<TCB> zombieQ;

TCB* active[] = {nullptr,nullptr,nullptr,nullptr}; // current TCB

extern "C" void contextSwitch(uint32_t* saveArea, uint32_t* restoreArea);
extern "C" void switchReg2Me(uint32_t* saveArea);
extern "C" void switchCand2Reg(uint32_t* restoreArea);

void thread_entry() {
    auto mycore=SMP::me();
    active[mycore]->do_your_thing();
    stop();
}


void stop() {
    auto candidate = readyQ.remove(); 
    while (candidate == nullptr) {
        asm ("pause");
        candidate = readyQ.remove();
    }
    while(candidate->save_area[5]==0){
        asm ("pause");
        readyQ.add(candidate);
        candidate = readyQ.remove();
        while (candidate == nullptr) {
            asm ("pause");
            candidate = readyQ.remove();
        }
    }
    auto mycore=SMP::me();
    auto me = active[mycore];
    active[mycore] = candidate; 
    zombieQ.add(me);

    contextSwitch(me->save_area, candidate->save_area); 
}


void threadsInit() {
    active[0] = new FakeTCB();
    active[1] = new FakeTCB();
    active[2] = new FakeTCB();
    active[3] = new FakeTCB();
}


void yield() {
    /* delete stopped threads */
    auto deadThreads = zombieQ.remove();
    while (deadThreads!=nullptr){
        delete deadThreads;
        deadThreads = zombieQ.remove();
    }

    auto mycore =SMP::me();
    auto candidate = readyQ.remove(); //candidate comes from the first block in readyQ

    if (candidate == nullptr) {
        return; // readyQ is empty
    }

    while(candidate->save_area[5]==0){
        asm ("pause");
        readyQ.add(candidate);
        candidate = readyQ.remove(); 
        if (candidate == nullptr) {
            return;
        }
    }
    // now : candidate->save_area[5]= 1; 
    auto me = active[mycore];
    active[mycore] = candidate;

    me->save_area[5]=0;
    readyQ.add(me); 
    contextSwitch(me->save_area, candidate->save_area); 
}