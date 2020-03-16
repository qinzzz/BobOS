#include "debug.h"
#include "threads.h"
#include "atomic.h"

/* Called by one CPU */
void kernelMain(void) {

    Atomic<int> waitingFor { 5 };

    for (int i=0; i<4; i++) {
        thread([&waitingFor] {
            waitingFor.add_fetch(-1);
            while (waitingFor.get() != 0) yield();
        });
    }

    waitingFor.add_fetch(-1);
    while (waitingFor.get() != 0) yield();

    Debug::printf("*** x = %d\n",waitingFor.get());

}

