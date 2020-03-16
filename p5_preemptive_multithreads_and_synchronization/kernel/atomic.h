#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include "machine.h"

extern "C" uint32_t disableInt();
extern "C" void restoreInt (uint32_t flag);
extern "C" uint32_t getFlags();

template <typename T>
class AtomicPtr {
    volatile T *ptr;
public:
    AtomicPtr() : ptr(nullptr) {}
    AtomicPtr(T *x) : ptr(x) {}
    AtomicPtr<T>& operator= (T v) {
        __atomic_store_n(ptr,v,__ATOMIC_SEQ_CST);
        return *this;
    }
    operator T () const {
        return __atomic_load_n(ptr,__ATOMIC_SEQ_CST);
    }
    T fetch_add(T inc) {
        return __atomic_fetch_add(ptr,inc,__ATOMIC_SEQ_CST);
    }
    T add_fetch(T inc) {
        return __atomic_add_fetch(ptr,inc,__ATOMIC_SEQ_CST);
    }
    void set(T inc) {
        return __atomic_store_n(ptr,inc,__ATOMIC_SEQ_CST);
    }
    T get(void) {
        return __atomic_load_n(ptr,__ATOMIC_SEQ_CST);
    }
    T exchange(T v) {
        T ret;
        __atomic_exchange(ptr,&v,&ret,__ATOMIC_SEQ_CST);
        return ret;
    }
};

template <typename T>
class Atomic {
    volatile T value;
public:
    Atomic(T x) : value(x) {}
    Atomic<T>& operator= (T v) {
        __atomic_store_n(&value,v,__ATOMIC_SEQ_CST);
        return *this;
    }
    operator T () const {
        return __atomic_load_n(&value,__ATOMIC_SEQ_CST);
    }
    T fetch_add(T inc) {
        return __atomic_fetch_add(&value,inc,__ATOMIC_SEQ_CST);
    }
    T add_fetch(T inc) {
        return __atomic_add_fetch(&value,inc,__ATOMIC_SEQ_CST);
    }
    void set(T inc) {
        return __atomic_store_n(&value,inc,__ATOMIC_SEQ_CST);
    }
    T get(void) {
        return __atomic_load_n(&value,__ATOMIC_SEQ_CST);
    }
    T exchange(T v) {
        T ret;
        __atomic_exchange(&value,&v,&ret,__ATOMIC_SEQ_CST);
        return ret;
    }
};


/* Bad idea when interrupts are enaled. Why? */
class SpinLock {
    Atomic<bool> taken;
public:
    SpinLock() : taken(false) {}
    void lock(void) {
        while (taken.exchange(true));
    }
    void unlock(void) {
        taken.set(false);
    }
};



class InterruptSafeLock{
    Atomic<bool> taken;
    // bool was;
public:
    InterruptSafeLock():taken(false){}
    bool lock(void) {
        bool was = disable();
        while (taken.exchange(true)){
            enable(was);
            was = disable();
        }
        return was;
    }
    void unlock (bool was){
        taken.set(false);
        enable(was);
    }
};

#endif
