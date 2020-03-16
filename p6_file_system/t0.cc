#include "stdint.h"
#include "debug.h"
#include "ide.h"
#include "bobfs.h"

void kernelStart(void) {
}

void check(const char* name, Node* f) {
    if (f == nullptr) {
        Debug::printf("*** 0 %s is null\n",name);
        return;
    }

    if (f->isFile()) {
        Debug::printf("*** 0 %s is a file\n",name);
    }
    if (f->isDirectory()) {
        Debug::printf("*** 0 %s is a directory\n",name);
    }

    Debug::printf("*** 0 %s has %d bytes\n",name,f->getSize());
    Debug::printf("*** 0 %s has %d links\n",name,f->getLinks());
}

Node* find(Node* d, const char* name) {
    Node* f = d->findNode(name);
    check(name,f);
    return f;
}

void kernelMain(void) {

    Ide* disk = new Ide(3);

    BobFS* fs = BobFS::mount(disk);
    
    Node* root = BobFS::root(fs);
    
    check("root",root);

    root->dump("root");
    
    find(root,"xyz");
    find(root,"zztop");
    find(root,"abc");
        
    Node* d1 = find(root,"d1");
    if (d1 != nullptr) {
        find(d1,"abc2");
    }
   
    Node* d2 = find(root,"d2");
    if (d2 != nullptr) {
        auto abc = find(d2,"abc2");
        if (abc != nullptr) {
            auto data = new uint32_t[2000];
            auto cnt = abc->readAll(0,data,10000);
            Debug::printf("*** 0 read %d bytes\n",cnt);
            for (uint32_t i=0; i<1000; i++) {
                if (data[i] != 0) {
                    Debug::printf("*** 0 data[%d] == %d\n",i,data[i]);
                    break;
                }
            }
            Debug::printf("*** 0 found 1000 zero values\n");
            for (uint32_t i=1000; i<2000; i++) {
                if (data[i] != i-999) {
                    Debug::printf("*** 0 data[%d] == %d\n",i,data[i]);
                    break;
                }
            }
            Debug::printf("*** 0 found 1000 sequential values\n");
        }
    }

    Debug::printf("*** 0 done\n");
    
    ideStats();
 
}

void kernelTerminate(void) {
}
