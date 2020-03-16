#include "stdint.h"
#include "heap.h"
#include "random.h"
#include "smp.h"
#include "debug.h"

/*
This test case trys to malloc i bit(s) (i=0,1,2,...,1299) continuously to reach the max size of the heap ( 1+2+..+1300 = 845,650 < 1,048,576 = 2^20),
and see if the heap can use the space efficiently. 
To test the merge function, in each loop it mallocs some larger size and free them.
To test if a user can store some data into the heap without errors, this case trys to store a string in different locations in the heap, and print them before freeing them.
It also covers the edge case when mallocing 0 bit or mallocing a very large number(heapsize-100).
*/

void kernelMain(void) {
    uint32_t me = SMP::me();
    if (me == kConfig.totalProcs-1) {
        //store a string into the heap
        char mystr[] ="*** Heap is my best friend!\n";
        int sz=sizeof(mystr)/sizeof(char);
        char* x[1300];
        for (int i=0; i< 1300; i++){
            char* y = (char*)malloc(i*10);
            x[i] = new char[i];

            if(i%500==100){
                for (int j=0; j<sz; j++)
                    x[i][j]=mystr[j];
            }
            free(y);
        }
        for (int i=0; i<1300; i++){
            if(i%500==100){
                Debug::printf("%s", x[i]);
            }
            free(x[i]);
        }

        // large number case
        int heapsize = 1<<20;
        char* p = (char*)malloc(heapsize-100);
        // Debug::printf("p: %p\n", p);
        free(p);

        Debug::shutdown();
    }
    else {
       while (true) {
           int sz = me;
           char* p = new char[sz];
           p[sz-1] = 0x66;
           p[0] = 0x66;
           free(p);
       }
    }
}

