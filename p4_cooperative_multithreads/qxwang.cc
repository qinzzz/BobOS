/*
    This test case tests if nested threads can work properly.
    If the program can run through all the threads successfully and reach the correct number, this test case is passed.
    By calling stop(), mycount is added but yieldTimes is not increased, so the output tests if the stop function works.
    It also tests the use of malloc and free operation in threads.
*/

#include "stdint.h"
#include "smp.h"
#include "debug.h"
#include "config.h"
#include "threads.h"

#define N 1000

/* Called by one CPU */
void kernelMain(void) {
    Atomic<int> mycount{0};
    Atomic<int> yieldTimes{0};
    char* area[N];

    // add 1 to mycount for 3*N times, 
    // add 1 to yieldTimes for 2*N times,
    // malloc and free 3*N times.
    thread([&mycount, &yieldTimes, &area](){
        for (int j = 0; j < N; j++) {
            mycount.fetch_add(1);
            area[j] = (char*)malloc(1);
            //nested
            thread([&mycount, &yieldTimes, &area, &j]{
                mycount.fetch_add(1);
                //nested
                thread([&mycount, &yieldTimes, &area, &j]{
                    mycount.fetch_add(1);
                    stop();
                    // should not continue
                    yieldTimes.fetch_add(1);
                    yield();
                });
                yieldTimes.fetch_add(1);
                yield();
            });
            yieldTimes.fetch_add(1);
            yield();
            free(area[j]);
        }
    });

    while(mycount!= 3*N) yield();

    Debug::printf("*** yields %d times\n", yieldTimes);
    Debug::printf("*** done\n");
}

