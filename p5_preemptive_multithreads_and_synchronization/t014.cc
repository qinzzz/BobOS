#include "debug.h"
#include "threads.h"
#include "barrier.h"

/**
This tests that the semaphores are consistent with the sum value
that gets added to every time we sync() which calls up and down.
If you output inconsistent values or if you get stuck in a lock
that will infinitely loop, you will fail this test.
**/

void kernelMain(void) {
	//tests 15 times 
	for (int j = 0; j < 15; j++) {
	    auto bar = new Barrier(7);
	    Atomic<int> sum{0};

	    for (int i = 0; i < 6; i++) {
		thread([bar,i,&sum] {
		    sum.add_fetch(i + 1);
		    bar->sync();
		});
	    }

	    bar->sync();
	    //sum should be 21 at this point every time 
	    Debug::printf("*** sum = %d\n",sum.get());
	}

}
