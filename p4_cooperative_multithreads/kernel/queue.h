#ifndef _queue_h_
#define _queue_h_

#include "atomic.h"

template <typename T>
class Queue {
    T* first = nullptr;
    T* last = nullptr;
    SpinLock lock;
public:
    Queue() : first(nullptr), last(nullptr), lock() {}

    void add(T* t) {
        lock.lock();
        t->next = nullptr;
        if (first == nullptr) {
            first = t;
        } else {
            last->next = t;
        }
        last = t;
        lock.unlock();
    }

    void addFront(T* t) {
        lock.lock();
        t->next = first;
        first = t;
        if (last == nullptr) last = first;
        lock.unlock();
    }

    T* remove() {
        lock.lock();
        if (first == nullptr) {
            lock.unlock();
            return nullptr;
        }
        auto it = first;
        first = it->next;
        if (first == nullptr) {
            last = nullptr;
        }
        lock.unlock();
        return it;
    }
};

#endif
