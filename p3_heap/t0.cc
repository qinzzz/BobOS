#include "stdint.h"
#include "heap.h"
#include "random.h"
#include "smp.h"
#include "debug.h"

uint32_t mCount = 0;
uint32_t fCount = 0;


constexpr uint32_t M = 20;
constexpr uint32_t N = 100000;

void kernelMain(void) {
    uint32_t me = SMP::me();
    if (me == kConfig.totalProcs-1) {
        Random* random = new Random(me);

        char* last[M];

        for (uint32_t i=0; i<M; i++) {
            last[i] = nullptr;
        }

        for (uint32_t i=0; i<N; i++) {
            uint32_t x = random->next() % M;
            if (last[x] != nullptr) {
                free(last[x]);
                fCount ++;
            }
            size_t sz = (size_t) random->next() % 1000;
            last[x] = (char*)malloc(sz+1);
            last[x][sz] = 0x77;
            mCount ++;
            if (last[x] == 0) {
                Debug::printf("*** failed to allocate %d\n",sz);
            }
        }

        for (uint32_t i=0; i<M; i++) {
            if (last[i] != 0) {
                free(last[i]);
                fCount ++;
            }
        }

        if (mCount != fCount) {
            Debug::printf("*** mCount %d\n",mCount);
            Debug::printf("*** fCount %d\n",fCount);
        } else {
            Debug::printf("*** count match\n");
        }

        if (mCount != N) {
            Debug::printf("*** wrong count %d\n",mCount);
        } else {
            Debug::printf("*** count ok\n");
        }

        Debug::shutdown();
    } else {
       while (true) {
           int sz = me + 100;
           char* p = new char[sz];
           p[sz-1] = 0x66;
           p[0] = 0x66;
           free(p);
       }
    }
}

