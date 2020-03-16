#ifndef _threads_h_
#define _threads_h_

#include "atomic.h"
#include "queue.h"
#include "heap.h"
#include "debug.h"

extern void threadsInit();
extern void stop();
extern void yield();
extern void thread_entry();

//virtual class
class TCB {
public:
    uint32_t save_area[6];  // the context of its registers from the last time it ran
                            // save_area[5] -- bool represents ready(1) or not(0)
    uint32_t* stack;
    TCB* next = nullptr;
    void virtual do_your_thing() = 0;
    virtual ~TCB(){}
};


class FakeTCB : public TCB {
public:
    FakeTCB(){
        save_area[5] = 1;
        // Debug::printf("fake!");
    }
    void do_your_thing(){
        // Debug::printf("I have nothing to do!");
    }
    ~FakeTCB(){}
};


// TCBImpl: 
template <typename T>
class TCBImpl : public TCB {
    T work;
    uint32_t* stack;
public:
    TCBImpl(T work): work(work) {
        stack = (uint32_t*) malloc(2048 * 4); // allocate a new stack for a thread
        stack[2047] = (uint32_t) thread_entry; // put thread_entry to the bottom of stack
        save_area[1] = (uint32_t) &stack[2047]; // ESP: set to the thread entry
        save_area[5] = 1;
    }
    void do_your_thing() {
       work();
    }
    ~TCBImpl(){
        free(stack);
    }
};

extern Queue<TCB> readyQ;


template <typename T>
void thread(T work) {
    auto tcb = new TCBImpl<T>(work);
    readyQ.add(tcb);
}


#endif
