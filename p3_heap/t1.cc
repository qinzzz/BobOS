#include "stdint.h"
#include "heap.h"
#include "random.h"
#include "smp.h"
#include "debug.h"

uint32_t total = 0;

void* get(uint32_t sz) {
    char* p = (char*)malloc(sz+1); 
    if (p != nullptr) {
        p[0] = 0x33;
        p[sz] = 0x44;
        total += sz+1;
    }
    return p;
}

constexpr uint32_t expected = 100000000;

void kernelMain(void) {
    auto me = SMP::me();

    if (me == kConfig.totalProcs-1) {
        Random random { 1000 };
        while (true) {
            void* p = get(random.next() % 60);
            if (p == nullptr) break;
            p = get(4000);
            if (p == nullptr) break;
            free(p);
        }
        if (total > expected) {
            Debug::printf("*** total above %d\n",expected);
        } else {
            Debug::printf("*** total is too low %d < %d\n",total,expected);
        }
        Debug::shutdown();
    } else {
       while (true) {
           int sz = me + 100;
           char* p = new char[sz];
           p[sz-1] = 0x66;
           p[0] = 0x55;
           free(p);
       }
    }
}

