#include "debug.h"
#include "smp.h"
#include "threads.h"
#include "atomic.h"
#include "future.h"


Queue<TCB> readyQ;
Queue<TCB> zombieQ;
Queue<TCB> blockQ;
static InterruptSafeLock ISL;
TCB* active[] = {nullptr,nullptr,nullptr,nullptr}; // current TCB

extern "C" void contextSwitch(uint32_t* saveArea, uint32_t* restoreArea);
extern "C" void switchReg2Me(uint32_t* saveArea);
extern "C" void switchCand2Reg(uint32_t* restoreArea);

void thread_entry() {
    auto me = active[SMP::me()];
    sti();
    me->do_your_thing();
    stop();
}

void stop() {
    disable(); //disable during the whole process

    auto candidate = readyQ.remove(); 
    auto mycore=SMP::me();
    auto me = active[mycore];

    while (candidate == nullptr) {
        asm ("pause");
        candidate = readyQ.remove();
    }
    while(candidate->save_area[5]==0 ){
        asm ("pause");
        readyQ.add(candidate);
        candidate = readyQ.remove();
        while (candidate == nullptr) {
            asm ("pause");
            candidate = readyQ.remove();
        }
    }

    active[mycore] = candidate; 
    me->save_area[5]=0;
    candidate->save_area[5]=0;
    zombieQ.add(me);
   
    // Debug::printf("zombie|%p\n", me);
    contextSwitch(me->save_area, candidate->save_area);
    // will not return here.
}


void threadsInit() {
    for (int i=0;i<4;i++){
        auto fake= new FakeTCB();
        active[i] = fake;
        fake->save_area[6] = 1; 
        // Debug::printf("Fake %p\n", fake);
    }

}

void yield() {
    /* print Q */
    // auto start = readyQ.first;
    // while(start){
    //     Debug::printf("%p->", start);
    //     start = start->next;
    // }
    // Debug::printf("end\n");
    /* print Q */

    // disable interrupt during the whole process
    // without interrupt, current thread will not be added to readyQ twice.
    bool was = disable(); 
    auto mycore =SMP::me();
    auto me = active[mycore];
    
    /* delete stopped threads */
    auto deadThreads = zombieQ.remove();
    while (deadThreads!=nullptr){
        // ISL.lock();
        if (deadThreads->save_area[5]==0){
            // Debug::printf("add|%p\n", deadThreads);
            zombieQ.add(deadThreads);
            break;
        }
        else{
            // Debug::printf("delete|%p\n", deadThreads);
            delete deadThreads;
        }
        deadThreads = zombieQ.remove();
        // ISL.unlock();
        // break;
    }

    auto candidate = readyQ.remove(); //candidate coms from the first block in readyQ

    if (candidate == nullptr) {
        return; // readyQ is empty
    }

    if (candidate->save_area[5]==0) {
        readyQ.add(candidate);
        return;
    }

    // now : candidate->save_area[5]= 1; 
    active[mycore] = candidate;

    me->save_area[5]=0;
    candidate->save_area[5]=0;
    readyQ.add(me); 
    
    contextSwitch(me->save_area, candidate->save_area); 

    // When leave the readyQ and run again from here, enable interrupt (may run on a different core).
    enable(was);
}