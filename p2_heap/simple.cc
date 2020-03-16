#include "stdint.h"
#include "heap.h"
#include "random.h"
#include "smp.h"
#include "debug.h"

uint32_t mCount = 0;
uint32_t fCount = 0;


constexpr uint32_t M = 20;
constexpr uint32_t N = 300;

void kernelMain(void) {
    uint32_t me = SMP::me();
    if (me == kConfig.totalProcs-1) {
        char* a = (char*)malloc(15);
        Debug::printf("a:%p\n", a);
        char* b = (char*)malloc(10);
         Debug::printf("b:%p\n", b);
        char* c = (char*)malloc(1);
         Debug::printf("c:%p\n", c);
        free(b);
        free(a);
        char* d = (char*)malloc(8);
         Debug::printf("d:%p\n", d);
        free(c);
        free(d);
        Debug::shutdown();
    } else {
        Debug::printf("begin\n");
        while (true) {
        //    int sz = me + 100;
        //    char* p = new char[sz];
        //    p[sz-1] = 0x66;
        //    p[0] = 0x66;
        //    free(p);
       }
       Debug::printf("end\n");
    }
}


