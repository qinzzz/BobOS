#include "stdint.h"
#include "debug.h"
#include "ide.h"
#include "bobfs.h"
#include "heap.h"
#include "libk.h"
/*
In this test case, John and Paul are trying to access the same disk and do their own operations.
Their operations (e.g. create a directory, create a file) should be visible to others. 
They want to contribute to the same song together.
Liam comes after and wants to read their songs, also adds his improvisation.
Finally, I want to test whether the file can reach the largest size, use up all the blocks.

The test case tests follow cases:
    Make BobFS on a disk
    Mount BobFs from a disk
    Create directories and files
    link a file
    Write big and incontinuous files
    Edit a file by different users
    Read previous files and directorys correctly
*/


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

}

Node* find(Node* d, const char* name) {
    Node* f = d->findNode(name);
    check(name,f);
    return f;
}
void kernelMain(void) {
    {
        Ide* disk = new Ide(3);

        BobFS* John = BobFS::mkfs(disk);
        Node* rootJ = BobFS::root(John);

        auto d1 = rootJ->newDirectory("Sgt.Peppers");
        auto d1f1 = d1->newFile("a_day_in_life");
        //auto d1f2 = d1->newFile("lonely_hearts");
        
        BobFS* Paul = BobFS::mount(disk);
        Node* rootP = BobFS::root(Paul);
        // find(rootP,"Sgt.Peppers");

        auto d2 = rootP->newDirectory("Help!");
        d2->linkNode("another_day_in_life",d1f1);
        auto d2f1 = d2->findNode("another_day_in_life");
       
        // John and Paul cooperates to write a file:
        char Johnptr[] = "*** I saw a film today, oh boy\n*** The English Army had just won the war\n*** A crowd of people turned away\n*** But I just had to look\n*** Having read the book\n*** love to turn you on...\n";
        uint32_t len = K::strlen(Johnptr);
        d1f1->writeAll(1500,Johnptr,len);

        char Paulptr = 'x';
        for (int i=0;i<160;i+=10){
            d2f1->writeAll(1500+i,&Paulptr,1);  
        }
        
    }

    {   // test Mount() ,Read() and add to a file.
        Ide* disk = new Ide(3);
        BobFS* Liam = BobFS::mount(disk);
        Node* rootL = BobFS::root(Liam);
        

        Node* LiamD2 = find(rootL,"Sgt.Peppers");
        auto LiamSong = find(LiamD2,"a_day_in_life");
        char Liamptr[] = "*** 1and2and3and4\n*** DuDuDuDuDu";
        LiamSong->writeAll(0, Liamptr, 50);

        // Liam wants to read the file:
        char* read = (char*)malloc(200);
        int cnt = LiamSong->readAll(1500, read, 300);
        Debug::printf("*** Liam sings -- %s\n",read);
        Debug::printf("*** Liam reads %d byte\n",cnt);
        
        char* add = (char*)malloc(50);
        LiamSong->readAll(0, add, 50);
        Debug::printf("*** Liam writes -- %s\n",add);

        
    }
    {   // test the largest capacity of file
        Ide* disk = new Ide(3);
        BobFS* me = BobFS::mount(disk);
        Node* rootMe = BobFS::root(me);
        
        auto longFile = rootMe->newFile("LongLong");
        for(uint32_t i =0;i<=263;i++){
            longFile->writeAll(i*1000, &i, 4);
        }
        rootMe->dump("root");
    }

    Debug::printf("*** 0 done\n");
}