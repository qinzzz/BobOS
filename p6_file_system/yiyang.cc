#include "stdint.h"
#include "debug.h"
#include "ide.h"
#include "bobfs.h"

typedef Node* NodePtr;

/**
 *To pass this test you have to account for several things
 *  1) Make sure that you can properly store indirect pointers to your
 *  tables. Since I have 100 files of total order 100^2 length this will test your table
 *  2) Make sure that you aren't wasteful with blocks. I write to a high offset
 *  but only a small amount so that one block is required to represent the file.
 *  Storing up to 256 blocks would be wasteful and you will probably run out of blocks.
 *  3) When you test name equivilance make sure that the lengths are equal. If one
 *  string is a prefix of the other then the streq could incorrectly return true
 *  4) Even though I don't explicitly write to middle sections of certain files,
 *  make sure that your code returns 0s for those values as it is implicit that
 *  those sections are zerod out (according to Piazza and t1).
 *
 *  If all of the above are implemented correctly you will probably pass.
 */


char* makeName(int val){
    auto str = new char[val+1];
    for (int i = 0; i < val; i++)
        str[i] = '$';
    str[val] = 0;
    return str;
}

void kernelMain(void) {
    Ide* disk = new Ide(3);

    BobFS* fs = BobFS::mkfs(disk);
    Node* root = BobFS::root(fs);
    NodePtr* nodes = new NodePtr[100];

    auto arr = new uint32_t[100];
    for (int i = 0; i < 100; i++)
        arr[i] = 37*i;

    for (uint32_t i = 0; i < 100; i++){
        nodes[i] = root->newFile(makeName(100-i));
        nodes[i]->writeAll(1024*256+42+i, &(arr[i]), 4);
    }

    //I'll be reading at blocks 1-100 and they should all be 0
    for (uint32_t i = 0; i < 100; i++){
        uint32_t val = 30;
        nodes[i]->readAll(1024*i+i, &val, 4);
        if (val != 0){
            Debug::printf("*** not zeroing out properly\n");
            break;
        }
    }

    for (uint32_t i = 0; i < 100; i++){
        uint32_t val = 13;
        nodes[i]->readAll(1024*256+42+i, &val,4);
        if (val != 37*i){
            Debug::printf("*** val is %d i is %d\n", val, i);
            Debug::printf("*** not dealing with indirect pointers properly\n");
            break;
        }
    }

    Node* twoDolla = root->findNode("$$");
    uint32_t val = 5;
    twoDolla->readAll(1024*256+42, &val, 4);
    if (val != 37*98)
        Debug::printf("Make sure lengths match up before reading file\n");

    Debug::printf("*** Cash Money\n");
    root->dump("root");
}

