#ifndef _queue_h_
#define _queue_h_

#include "atomic.h"

template <typename T>
class Queue {
public:
    T* first = nullptr;
    T* last = nullptr;
    InterruptSafeLock lock;
public:
    Queue() : first(nullptr), last(nullptr), lock() {}

    void add(T* t) {
        bool was = lock.lock();
        t->next = nullptr;
        if (first == nullptr) {
            first = t;
        } else {
            last->next = t;
        }
        last = t;
        lock.unlock(was);
    }

    void addFront(T* t) {
        bool was = lock.lock();
        t->next = first;
        first = t;
        if (last == nullptr) last = first;
        lock.unlock(was);
    }

    T* remove() {
        bool was = lock.lock();
        if (first == nullptr) {
            lock.unlock(was);
            return nullptr;
        }
        auto it = first;
        first = it->next;
        if (first == nullptr) {
            last = nullptr;
        }
        lock.unlock(was);
        return it;
    }
};

#endif
