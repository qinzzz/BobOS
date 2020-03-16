#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "stdint.h"
#include "atomic.h"
#include "queue.h"
#include "heap.h"
#include "debug.h"
#include "threads.h"

extern TCB* active[];
extern Queue<TCB> readyQ;

class Semaphore {
	Atomic<uint64_t> counter;
	Queue<TCB> blockQ;
	InterruptSafeLock ISL;
public:
	Semaphore(const uint32_t count):counter(count){};
	void down();
	void up();
	void block(bool was);
};


#endif

