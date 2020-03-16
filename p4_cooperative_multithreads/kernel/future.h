#ifndef _future_h_
#define _future_h_

template <typename T>
class Future {
    bool isReady;
    T    t;
public:
    Future() : isReady(false), t() {}
    void set(T v) {
        t = v;
        asm volatile("sfence" : : : "memory");
        isReady = true;
    }
    T get() {
        while (!isReady) yield();
        asm volatile("lfence" : : : "memory");
        return t;
    }
};



#endif

