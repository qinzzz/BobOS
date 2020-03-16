#include "semaphore.h"
#include "debug.h"
#include "smp.h"
#include "atomic.h"
#include "threads.h"

// static InterruptSafeLock ISL;
extern "C" void contextSwitch(uint32_t* saveArea, uint32_t* restoreArea);

void Semaphore::block(bool was) {
    auto candidate = readyQ.remove(); 
    auto mycore=SMP::me();
    auto me = active[mycore];
    ISL.unlock(was);
    // while(me->save_area[5]==0);

    while (candidate == nullptr) { // problem
        // maybe someone call UP()
        was = ISL.lock();
        if(counter.get()>0){
            counter.add_fetch(-1);
            ISL.unlock(was);
            return;
        }
        ISL.unlock(was);
        candidate = readyQ.remove();
        // Debug::printf("w");
    }

    while(candidate->save_area[5]==0){
        readyQ.add(candidate);
        candidate = readyQ.remove();
        while (candidate == nullptr) {
            // maybe someone call UP()
            was = ISL.lock();
            if(counter.get()>0){
                counter.add_fetch(-1);
                ISL.unlock(was);
                return;
            }
            ISL.unlock(was);
            candidate = readyQ.remove();
        }
    }
    
    was = ISL.lock();
    if(counter.get()>0){
        counter.add_fetch(-1);
        readyQ.add(candidate);
        ISL.unlock(was);
        return;
    }
    
    active[mycore] = candidate; 
    me->save_area[5]=0;
    candidate->save_area[5]=0;
    blockQ.add(me);
    ISL.unlock(false); // unlock but not enable interrupt.

    contextSwitch(me->save_area, candidate->save_area); 
    enable(was);
}


void Semaphore::up(void) {
    bool was = ISL.lock();
    auto candidate = blockQ.remove();
    if (candidate==nullptr){
        counter.add_fetch(1);
    } 
    else {
        // Debug::printf("add to readyQ %p\n", candidate);
        readyQ.add(candidate);
    }
    ISL.unlock(was);
}


void Semaphore::down(void) {
    bool was = ISL.lock();
	if (counter.get()==0){
        block(was); 
	}
	else{
		counter.add_fetch(-1);
        ISL.unlock(was);
	}
}
