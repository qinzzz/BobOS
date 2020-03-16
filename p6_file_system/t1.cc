#include "stdint.h"
#include "debug.h"
#include "ide.h"
#include "bobfs.h"

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
    {
        Ide* disk = new Ide(3);

        BobFS* fs = BobFS::mkfs(disk);
        Node* root = BobFS::root(fs);
        
        auto f1 = root->newFile("the_who");
        auto f2 = root->newFile("deep_purple");
        auto f3 = root->newFile("pq");
        
        auto d1 = root->newDirectory("d1");

        d1->linkNode("was_the_who",f1);
        d1->linkNode("was_deep_purple",f2);
        
        auto d2 = root->newDirectory("d2");
        d2->linkNode("was_pq",f3);

        auto ptr = new uint32_t[1000];
        for (int i=0; i<500; i++) {
            ptr[i] = i+1;
        }

        f1->writeAll(4000,ptr,2000);

        d2->linkNode("the_who2",f1);

        root->dump("root");
    }

    {
        Ide* disk = new Ide(3);

        BobFS* fs = BobFS::mount(disk);
        Node* root = BobFS::root(fs);

        check("root",root);

        root->dump("root");

        find(root,"deep_purple");
        find(root,"zztop");
        find(root,"the_who");
        
        Node* d1 = find(root,"d1");
        if (d1 != nullptr) {
            find(d1,"the_who2");
        }

   
        Node* d2 = find(root,"d2");
        if (d2 != nullptr) {
            auto the_who = find(d2,"the_who2");
            if (the_who != nullptr) {
                auto data = new uint32_t[2000];
                auto cnt = the_who->readAll(0,data,10000);
                Debug::printf("*** 0 read %d bytes\n",cnt);
                for (uint32_t i=0; i<1000; i++) {
                    if (data[i] != 0) {
                        Debug::printf("*** 0 data[%d] == %d\n",i,data[i]);
                        // break;
                    }
                }
                Debug::printf("*** 0 found 1000 zero values\n");
                for (uint32_t i=1000; i<1500; i++) {
                    if (data[i] != i-999) {
                        Debug::printf("*** 0 data[%d] == %d\n",i,data[i]);
                        break;
                    }
                }
                Debug::printf("*** 0 found 1000 sequential values\n");
            }
        }
    }

    Debug::printf("*** 0 done\n");

    ideStats();
 
}
