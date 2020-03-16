#include "atomic.h"


bool disable(){
    bool wasDisabled = (getFlags() & 0x200);
    cli();
    return wasDisabled;
}

void enable (bool flag){
    if (flag != 0)
        sti();
    else // flag is 0 -- previously disabled!
        return;
}
