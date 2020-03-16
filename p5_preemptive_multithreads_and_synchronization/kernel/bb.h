#ifndef _bb_h_
#define _bb_h_

#include "debug.h"
#include "semaphore.h"


template <int N, typename T>
class BoundedBuffer {
    T buffer[N];
    int next_in;
    int next_out;
    Semaphore empty{N};
    Semaphore data{0};
    Semaphore mutex{1};
    // InterruptSafeLock myISL;
public:
    BoundedBuffer() {
        next_in=0;
        next_out=0;
    }

    void put(T t) {
        empty.down();
        mutex.down();
        // myISL.lock();
        buffer[next_in] = t;
        next_in = (next_in +1) % N;
        mutex.up();
        // myISL.unlock();
        data.up();
    }

    T get() {
        data.down();
        mutex.down();
        // myISL.lock();
        T x = buffer[next_out];
        next_out = (next_out +1) % N;
        mutex.up();
        // myISL.unlock();
        empty.up();
        return x;
    }
};

#endif
