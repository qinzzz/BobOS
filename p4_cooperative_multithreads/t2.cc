#include "debug.h"
#include "threads.h"
#include "atomic.h"
#include "future.h"

/* Called by one CPU */
void kernelMain(void) {

    Future<int> answer;

    thread([&answer] {
        answer.set(42);
    });

    

    Debug::printf("*** answer = %d\n",answer.get());

}

