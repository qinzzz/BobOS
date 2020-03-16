#include "heap.h"
#include "debug.h"
#include "stdint.h"
#include "atomic.h"
#include "critical.h"

char* heapstart;
int heapsize;
static SpinLock s;


/*********************/
/* STRUCT DEFINITION */
/*********************/

template <typename T>
T* ptr_add(void* p, int offset){
    return (T*)(((char*)p)+offset);
}

struct Footer {
    int size;
    inline bool isfree(){return size<0;}
};

// |Header(4B)|
struct Header {
    int size;
    inline bool isfree(){return size<0;}
    struct Header* next(){
        struct Header* nexthead;
        if (size == 0)
            return nullptr;
        else if (size>0){
            nexthead=ptr_add<Header>(this, (size+2)*4);
        }
        else{
            nexthead= ptr_add<Header>(this, (-size+2)*4);
        }
        if(nexthead >= ptr_add<Header>(heapstart, heapsize) || nexthead <  (struct Header*)heapstart) // pass the boundry
            return nullptr;
        else
            return nexthead;
    }
    struct Header* prev(){
        struct Header* prevhead;
        struct Footer* f = ptr_add<Footer>(this, -sizeof(Header));
        if(f->size ==0){
            return nullptr;
        }
        else if (f->size>0)
            prevhead = ptr_add<Header>(this, -(f->size+2)*4);
        else
            prevhead = ptr_add<Header>(this, -(-f->size+2)*4);
        if(prevhead < (struct Header*)heapstart || prevhead >= ptr_add<Header>(heapstart, heapsize))
            return nullptr;
        else
            return prevhead;
    }
    inline struct Footer* findFooter(){
        if (size>0)
            return ptr_add<Footer>(this, (size+1)*4);
        else
            return ptr_add<Footer>(this, (-size+1)*4);
    }
    
};
 

// |Header(4B)|prev(4B)|next(4B)|
struct freeHeader{
    struct Header head;
    struct freeHeader* prev =nullptr;
    struct freeHeader* next =nullptr;
};

struct freeHeader* freehead;


/*******************/
/* DEBUG FUNCTIONS */
/*******************/


void printLinkedList(struct freeHeader* head){
    struct freeHeader* p = head;
    int count=0;
    while(p){
        count++;
        Debug::printf("%p(%d)->", p, p->head.size);
        p = p->next;
        if (count>100)
            while(true);
    }
    if(!p)
        Debug::printf("NULL");
    Debug::printf("\n");
}

bool checkfreenodes(){
    struct freeHeader* p = freehead;
    while(p){
        if (p->head.size>0 || p->head.size<-heapsize){
            Debug::printf("WRONG: %p(%d)",p,p->head.size);
            return false;
        }
        p=p->next;
    }
    return true;
}

bool goFromLeftToRight(){
    struct Header* p =(struct Header*)heapstart;
    struct Header* end = ptr_add<Header>(heapstart, heapsize);
    struct Footer* endfoot = ptr_add<Footer>(end, -sizeof(Footer));
    while(p && p->next() && p<end){
        p = p->next();
    }
    if (p->findFooter()!=endfoot)
        return false;
    return true;
}

bool printFromLeftToRight(){
    struct Header* p =(struct Header*)heapstart;
    struct Header* end = ptr_add<Header>(heapstart, heapsize);
    struct Footer* endfoot = ptr_add<Footer>(end, -sizeof(Footer));
    while(p && p->next() && p<end){
        Debug::printf("%p(%d)|",p,p->size);
        p = p->next();
    }
    Debug::printf("%p(%d)\n",p,p->size);
    if (p->findFooter()!=endfoot){
        Debug::printf("Last: %p(%d)\n",p,p->size);
        return false;
    }
    return true;
}


/********************/
/* HELPER FUNCTIONS */
/********************/

void merge(struct freeHeader* p){

    struct Header* right = p->head.next();
    struct Header* left = p->head.prev();
    if (right && right->isfree()){
        struct freeHeader* freeright = (struct freeHeader*) right;
        p->head.size = p->head.size + freeright->head.size-2; 
        struct Footer* pfoot = p->head.findFooter();
        pfoot->size = p->head.size;

        if(!freeright -> next){
            freeright->prev->next = nullptr;
        }
        else if(!freeright -> prev){
            freeright->next->prev = nullptr;
            freehead = freeright->next;
        }
        else{
            freeright->next->prev = freeright->prev;
            freeright->prev->next = freeright->next;
            
        }
        // if (p<(void *)0x1fffff){
        //     Debug::printf("merge right: %p(%d) and %p(%d)\n", p,p->head.size, freeright, freeright->head.size);
        // }
    }
    if (left && left->isfree()){
        struct freeHeader* freeleft = (struct freeHeader*) left;
        freeleft->head.size = freeleft->head.size + p->head.size-2; 
        struct Footer* freeleftfoot = freeleft->head.findFooter();
        freeleftfoot->size = freeleft->head.size;
        
        if(!p -> next){
            p->prev->next = nullptr;
        }
        else if(!p -> prev){
            p->next->prev = nullptr;
            freehead = p->next;
        }
        else{
            p->prev->next = p->next;
            p->next->prev = p->prev;
            
        }
        // if (p<(void *)0x1ccccc){
        //     Debug::printf("merge left: %p(%d) and %p(%d)\n", freeleft, freeleft->head.size, p, p->head.size);
        // }
    }
}



void heapInit(void* start, size_t bytes) {
    heapstart = (char*)start; // not move
    heapsize = bytes; // not change
    
    // |Header=(heapsize/4 - 2)|prev=null|next=null|                    |Footer=(heapsize/4 - 2)|
    // store size in the header by words (bytes/4)

    // head of free list
    freehead = (struct freeHeader*)start;
    freehead->next = nullptr;
    freehead->prev= nullptr;
    freehead->head.size = -((heapsize>>2) - 2);
    // foot of free list
    struct Footer* freefoot = freehead->head.findFooter();
    freefoot->size = freehead->head.size;

    Debug::printf("init header:%p\n", freehead);
    Debug::printf("init size:%d\n", freehead->head.size);
    Debug::printf("init footer:%p\n", freefoot);
    goFromLeftToRight();
}

void* malloc(size_t bytes) {
    s.lock();
    //Debug::printf("--------malloc %d--------\n", bytes);
    
    if ((int)bytes > heapsize){
        s.unlock();
        return nullptr;
        }
    /* 
    go through freenode list, find the first one 
    allocate memory from the end of free block
    */
    struct freeHeader* finder = freehead;
    
    // malloc(0): return a random pointer
    struct Header* end = ptr_add<Header>(heapstart, heapsize);
    if (bytes == 0){
        s.unlock();
        return (void*)end;
    }

    // round the allocate size to n*4B
    int wordSize = (bytes%4==0)? (bytes>>2) : (bytes>>2)+1;

    if (wordSize<2)
        wordSize=2;

    // find a freenode to allocate 
    int counter = 0;
    while(finder && (-(finder->head.size) < wordSize) ){
        // if(counter>1000){
        //     finder = nullptr;
        //     break;
        // }
        finder = finder -> next;
        counter++;
    }
    // reach the end
    if(finder == nullptr){
        s.unlock();
        return nullptr;
    }
    // newHead: the head of the return pointer
    struct Header* newHead;

    // ALLOCATE THE WHOLE FREENODE 
    // DON'T FORGET TO DELETE THE FREENDOE FROM LIST!!!!!!
    if(-(finder->head.size) - wordSize <= 4){
        newHead = (struct Header*)finder;
        newHead->size = -(finder->head.size);
        // Footer also become positive!
        struct Footer* newFoot = newHead->findFooter();
        newFoot->size= newHead->size;

        if(!finder->prev){
            freehead = finder->next;
            finder->next->prev=nullptr;
        }
        else if(!finder->next){
            finder->prev->next = nullptr;
        }
        else{
            finder->next->prev = finder -> prev;
            finder->prev->next = finder -> next;
        }
    }
    else{ // block size >= 4 words

        // split the finder node, put the allocated block at the right part;
        // finder : the first freenode that is large enough

        // change the freenode size
        finder->head.size = (finder->head.size)+wordSize+2;
        // finder should have a new footer!
        struct Footer* finderFoot = finder->head.findFooter();
        finderFoot->size = finder->head.size;

        newHead = finder->head.next();
        newHead->size = wordSize;
        struct Footer* newFoot = newHead->findFooter();
        newFoot->size = wordSize;

    }

    // p : the return pointer = newHead + 4B
    void* p = ptr_add<char>(newHead, sizeof(struct Header));
    
    // check invariances
   
    // if(!checkfreenodes()){
    //     Debug::printf("wrong freenodes\n");
    //     printLinkedList(freehead);
    //     while(true);
    // }
    // if (!goFromLeftToRight()){
    //     Debug::printf("wrong heads\n");
    //     printFromLeftToRight();
    //     //while(true);
    // }
    
    // if (p<(void *)0x1000ff){
    //     Debug::printf("free list:\n");
        // printLinkedList(freehead);
    //     //Debug::printf("heap:\n");
        // printFromLeftToRight();

    // }

    s.unlock();
    return p;
}

void free(void* p) {
    s.lock();

    //Debug::printf("---free----- %p\n", p);
    struct Header* end = ptr_add<Header>(heapstart, heapsize);
    if(!p || p == (void*)end){
        s.unlock();
        return;}


    // h is header of p
    auto h = ptr_add<struct Header>(p,-sizeof(struct Header));
    if (h->size <= 0){
        s.unlock();
        return;}

    // freehead cur have negative size!!
    struct freeHeader* cur = (struct freeHeader*)h;
    cur->head.size = -(h->size); 
    struct Footer* curfoot = cur->head.findFooter();
    curfoot->size = cur->head.size;

    // add to freenode list
    cur->next = freehead;
    cur->prev = nullptr;
    freehead->prev = cur;

    // freenode->next don't change
    freehead = cur;

    //Debug::printf("---free----- %p\n", p);
    // Debug::printf("size: %d, %d\n", cur->head.size, curfoot->size);
    //printLinkedList(freehead);

    //merge node
    merge(cur);
    
    // check invariances
    // if(!checkfreenodes()){
    //     Debug::printf("wrong freenodes\n");
    //     printLinkedList(freehead);
    //     printFromLeftToRight();
    //     //while(true);
    // }

    s.unlock();

}



/*****************/
/* C++ operators */
/*****************/

void* operator new(size_t size) {
    void* p =  malloc(size);
    if (p == 0) Debug::panic("out of memory");
    return p;
}

void operator delete(void* p) noexcept {
    return free(p);
}

void operator delete(void* p, size_t sz) {
    return free(p);
}

void* operator new[](size_t size) {
    void* p =  malloc(size);
    if (p == 0) Debug::panic("out of memory");
    return p;
}

void operator delete[](void* p) noexcept {
    return free(p);
}

void operator delete[](void* p, size_t sz) {
    return free(p);
}
