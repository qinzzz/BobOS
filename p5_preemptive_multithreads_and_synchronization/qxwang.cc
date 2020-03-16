/*
    This test case tests if 100 threads can work at the same time woth nested preemption threads.
    The inside threads call get() and down(), while the outside threads call put() and up().
    When we get to the sum we want, done is up and the program ends.
*/

#include "stdint.h"
#include "smp.h"
#include "debug.h"
#include "config.h"
#include "threads.h"
#include "bb.h"
#include "barrier.h"

#define N 50

/* Called by one CPU */
void kernelMain(void) {
    Atomic<int> mycount{0};
    Atomic<int> mysum{0};
    Semaphore sem{0}, done{0};
    BoundedBuffer<5,int> buffer{};
    auto blocked = new Barrier(N+1);

    thread([&mycount, &buffer, &done, &sem, &mysum, &blocked](){
        while(true){
            // get the right sum
            if (mysum >= 1275) {
                done.up();
                break;
            }
            thread([&mycount, &buffer, &blocked, &sem]{
                mycount.fetch_add(1);
                //nested
                thread([&mycount, &buffer, &sem]{
                    buffer.put(mycount.get());
                    sem.up();
                });
            });
            sem.down();
        }
        
    });

    for(int j=0; j< N;j++){
        int x = buffer.get();
        mysum.add_fetch(x);
        // try to see if yield() can work properly alone.
        yield();
    }

    done.down();

    Debug::printf("*** sum = %d\n", mysum);
    Debug::printf("*** done\n");
}

