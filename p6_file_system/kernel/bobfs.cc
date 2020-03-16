#include "bobfs.h"
#include "libk.h"
#include "heap.h"
#include "debug.h"
#include "machine.h"
#include "random.h"


uint8_t zero_1024[BobFS::BLOCK_SIZE];

//////////
// Node //
//////////

/*
i-node has 16 bytes:
     * 16 bit i-node type (1 directory, 2 file)
     * 16 bit nlinks
     * 32 bit size
     * 1 pointer to direct block
     * 1 pointer to indirect block
*/

Node::Node(BobFS* fs, uint32_t inumber) {
    myFS = fs;
    device = fs->device;
    this->inumber = inumber;
}


bool Node::isDirectory(void) {
    return getType() == DIR_TYPE;
}

bool Node::isFile(void) {
    return getType() == FILE_TYPE;
}

bool streq(const char* a, const char* b) {
    uint32_t i = 0;
    if (K::strlen(a)!=K::strlen(b))
        return false;
    while (true) {
        char x = a[i];
        char y = b[i];
        if (x != y) return false;
        if (x == 0) return true;
        i++;
    }
    return true;
}

Node* Node::findNode(const char* name) {
    /* fine the name in a directory */
    uint32_t size = this->getSize();

    if(this->isFile())
        return nullptr;
    if (size==0)
        return nullptr;

    uint32_t cur = 0;
    
    while (cur < size){
        uint32_t inumber, length;
        readAll(cur, &inumber, 4);
        readAll(cur+4, &length, 4);
        char *thisname = (char*)malloc(length+1);
        readAll(cur+8, thisname, length);
        thisname[length]=0;

        if (streq(name, thisname)){ // to change
            Node* pNode = new Node(myFS, inumber);
            return pNode;
        }
        cur = cur + 8 +length;
    }
    return nullptr;
}
    
uint16_t Node::getLinks(void) {
    uint16_t nlinks;
    device->readAll(BobFS::START_OF_INODES+16*inumber+2, &nlinks ,2);
    return nlinks;
}

uint32_t Node::getSize(void) {
    uint32_t size;
    device->readAll(BobFS::START_OF_INODES+16*inumber+4, &size ,4);
    return size;
}

uint32_t Node::getDirect(void) {
    uint32_t direct;
    device->readAll(BobFS::START_OF_INODES+16*inumber+8, &direct ,4);
    return direct;
}

uint32_t Node::getIndirect(void) {
    uint32_t indirect;
    device->readAll(BobFS::START_OF_INODES+16*inumber+12, &indirect ,4);
    return indirect;
}

uint32_t Node::getInumber(void) {
    return inumber;
}

uint16_t Node::getType(void) {
    uint16_t type;
    device->readAll(BobFS::START_OF_INODES+16*inumber, &type ,2);
    return type;
}

uint32_t Node::getTotalBlocks(){
    uint32_t n = getSize() / BobFS::BLOCK_SIZE;
    uint32_t offset = getSize()%BobFS::BLOCK_SIZE;
    if (offset >0)
        n++;
    return n; 
}

void Node::addLink(void){
    uint32_t nlinks = getLinks();
    nlinks++;
    device->writeAll(BobFS::START_OF_INODES+inumber*16+2, &nlinks, 2);
}

void Node::writeBlock(uint32_t blocktoWrite, const void* buffer){
    uint32_t totalBlocks = getTotalBlocks();
    uint32_t directBlock = getDirect();
    uint32_t indirectBlock = getIndirect();

    if(blocktoWrite >= totalBlocks){
        uint32_t newtotalBlocks = blocktoWrite + 1;
        uint32_t iNodeStart = BobFS::START_OF_INODES + 16 * inumber;
        // write new block number into inode
        if(directBlock == 0){
            directBlock = findEmptyBlock();
            device->writeAll(iNodeStart + 8, &directBlock, 4);
        }
        if (newtotalBlocks > 1){
            if (indirectBlock == 0){
                indirectBlock = findEmptyBlock();
                device->writeAll(iNodeStart + 12, &indirectBlock, 4);
            }
            // write block number to indirect block
            uint32_t x = findEmptyBlock();
            device->writeAll(indirectBlock * (BobFS::BLOCK_SIZE) + (blocktoWrite-1)*4, &x, 4);
        }
    }
    uint32_t blockIdx;
    if (blocktoWrite==0)
        blockIdx = directBlock;
    else
        device->readAll(indirectBlock * (BobFS::BLOCK_SIZE) + (blocktoWrite-1)*4, &blockIdx, 4);

    uint32_t blockOffset = blockIdx * BobFS::BLOCK_SIZE;
    device->writeAll(blockOffset, buffer, BobFS::BLOCK_SIZE);
    // free(blockIdx);
}

int32_t Node::write(uint32_t offset, const void* buffer, uint32_t n) {

    uint32_t blocktoWrite = offset / BobFS::BLOCK_SIZE;
    uint32_t start = offset % BobFS::BLOCK_SIZE;

    uint32_t end = start + n;
    if (end > BobFS::BLOCK_SIZE) end = BobFS::BLOCK_SIZE;

    uint32_t count = end-start; // bytes we really read 
    
    if(count == BobFS::BLOCK_SIZE){
        writeBlock(blocktoWrite, buffer);
    }
    else if(count!=0){
        char data[BobFS::BLOCK_SIZE]={0};
        // if we didn't write to this block before, we should clean it first!
        if (blocktoWrite >= getTotalBlocks()){
            memcpy(&data[start], buffer, count);
        }
        else{
            readBlock(blocktoWrite, data);
            memcpy(&data[start], buffer, count);
        }
        writeBlock(blocktoWrite,data); // allocate new blocks
    }
    uint32_t size = getSize();
    size = (offset+count>size)? offset+count : size;
    device->write(BobFS::START_OF_INODES + 16*inumber + 4, &size, 4);
    
    return count;
}

int32_t Node::writeAll(uint32_t offset, const void* buffer_, uint32_t n) {
    int32_t total = 0;
    char* buffer = (char*) buffer_;

    while (n > 0) {
        int32_t cnt = write(offset,buffer,n);
        if (cnt <= 0) return total;

        total += cnt;
        n -= cnt;
        offset += cnt;
        buffer += cnt;
    }
    return total;
}

void Node::readBlock(uint32_t blocktoRead, void* buffer){
    uint32_t blockIdx;
    if (blocktoRead==0)
        blockIdx = getDirect();
    else
        device->readAll(getIndirect() * (BobFS::BLOCK_SIZE) + (blocktoRead-1)*4, &blockIdx, 4);

    if (blockIdx==0){
        memcpy(buffer, zero_1024, BobFS::BLOCK_SIZE);
    }
    else{
        uint32_t blockOffset = blockIdx * BobFS::BLOCK_SIZE;
        device->readAll(blockOffset, buffer, BobFS::BLOCK_SIZE);
    }

}

int32_t Node::read(uint32_t offset, void* buffer, uint32_t n) {
    // Split reading by blocks 
    // 
    uint32_t size = getSize();

    if (offset == size) return 0;
    if (offset > size) return -1;
    if (offset + n > size) n = size-offset;

    uint32_t blocktoRead = offset / BobFS::BLOCK_SIZE;
    uint32_t start = offset % BobFS::BLOCK_SIZE;

    uint32_t end = start + n;
    if (end > BobFS::BLOCK_SIZE) end = BobFS::BLOCK_SIZE;

    uint32_t count = end-start; // bytes we really read 

    if(count == BobFS::BLOCK_SIZE){
        readBlock(blocktoRead, buffer);
    }
    else if(count!=0){
        char data[BobFS::BLOCK_SIZE];
        readBlock(blocktoRead, data);
        memcpy(buffer, &data[start], count);
    }
    return count;
}

int32_t Node::readAll(uint32_t offset, void* buffer_, uint32_t n) {
    uint32_t size = getSize();

    if (offset == size) return 0;
    if (offset > size) return -1;
    if (offset + n > size) n = size-offset;

    int32_t total = 0;
    char* buffer = (char*) buffer_;

    while (n > 0) {
        int32_t cnt = read(offset,buffer,n);
        if (cnt <= 0) return total;

        total += cnt;
        n -= cnt;
        offset += cnt;
        buffer += cnt;
    }
    return total;
}

void Node::linkNode(const char* name, Node* node) {
    if(this->isFile()) return;
    if(node->isDirectory()) return;

    node->addLink();
    uint32_t x = node->getInumber();
    uint32_t length = K::strlen(name);

    writeAll(this->getSize(), &x, 4);
    writeAll(this->getSize(), &length, 4);
    writeAll(this->getSize(), name, length);

}

void Node::initNewNode(const uint32_t inumber, const uint32_t type){
    uint16_t nlinks=1;
    uint32_t size=0;
    uint32_t directBlock=0;
    uint32_t indirectBlock=0;
    uint32_t start = BobFS::START_OF_INODES + 16*inumber;
    device->writeAll(start, &type, 2);
    device->writeAll(start+2, &nlinks, 2);
    device->writeAll(start+4, &size, 4);
    device->writeAll(start+8, &directBlock, 4);
    device->writeAll(start+12, &indirectBlock, 4);
}

Node* Node::newNode(const char* name, uint32_t type) {
    // returns null if the current Node is not a directory
    if(this->isFile()) return nullptr;
    
    // find an available node inumber in the bitmap
    Random random { inumber };
    uint32_t x; // inumber for the new node
    bool used;
    while(true){
        x = random.next() % (8*1024);
        used = getNodeBit(x);
        if(used == 0){
            setNodeBit(x, 1);
            break;
        }
    }
    // create a new node
    initNewNode(x, type);
    Node* pNode = new Node(this->myFS, x);

    // create an entry in the directory
    uint32_t length = K::strlen(name);
    writeAll(this->getSize(), &x, 4);
    writeAll(this->getSize(), &length, 4);
    writeAll(this->getSize(), name, length);
    return pNode;
}

Node* Node::newFile(const char* name) {
    return newNode(name, FILE_TYPE);
}

Node* Node::newDirectory(const char* name) {
    return newNode(name, DIR_TYPE);
}

void Node::dump(const char* name) {
// #if 0
    uint32_t type = getType();
    // Debug::printf("type:%d", type);
    switch (type) {
    case DIR_TYPE:
        Debug::printf("*** 0 directory:%s(%d)\n",name,getLinks());
        {
            uint32_t sz = getSize();
            // Debug::printf("dir size: %d", sz);
            uint32_t offset = 0;

            while (offset < sz) {
                uint32_t ichild;
                readAll(offset,&ichild,4);
                offset += 4;
                uint32_t len;
                readAll(offset,&len,4);
                offset += 4;
                char* ptr = (char*) malloc(len+1);
                readAll(offset,ptr,len);
                offset += len;
                ptr[len] = 0;              
                
                Node* child = Node::get(myFS,ichild); ////
                child->dump(ptr);
                free(ptr);
            }
        }
        break;
    case FILE_TYPE:
        Debug::printf("*** 0 file:%s(%d,%d)\n",name,getLinks(),getSize());
        break;
    default:
        Debug::panic("unknown i-node type %d\n",type);
    }
// #endif
}

bool Node::getBlockBit(uint32_t bitoffset){
    int byte = bitoffset / 8;
    int remainingBit = bitoffset % 8;
    uint32_t aByte;
    device->readAll(BobFS::BLOCK_SIZE + byte, &aByte, 1);
    return (aByte >> remainingBit)&1;
}

void Node::setBlockBit(uint32_t bitoffset, bool bit){
    int byte = bitoffset / 8;
    int remainingBit = bitoffset % 8;
    uint32_t aByte;
    device->readAll(BobFS::BLOCK_SIZE + byte, &aByte, 1);
    if (bit ==1)
        aByte |= 1<<remainingBit;
    else // bit == 0
        aByte &= ~(1 << remainingBit);
    device->writeAll(BobFS::BLOCK_SIZE + byte, &aByte, 1);
}

bool Node::getNodeBit(uint32_t bitoffset){
    int byte = bitoffset / 8;
    int remainingBit = bitoffset % 8;
    uint32_t aByte;
    device->readAll(2*BobFS::BLOCK_SIZE + byte, &aByte, 1);
    return (aByte >> remainingBit)&1;
}

void Node::setNodeBit(uint32_t bitoffset, bool bit){
    int byte = bitoffset / 8;
    int remainingBit = bitoffset % 8;
    uint32_t aByte;
    device->readAll(2*BobFS::BLOCK_SIZE + byte, &aByte, 1);
    if (bit==1) 
        aByte |= 1<<remainingBit;
    else // bit == 0
        aByte &= ~(1 << remainingBit);
    device->writeAll(2*BobFS::BLOCK_SIZE + byte, &aByte, 1);
}

uint32_t Node::findEmptyBlock(){
    // return block number
    Random random { inumber };
    uint32_t x; // inumber for the new node
    bool used;
    while(true){
        x = random.next() % (8*1024);
        used = getBlockBit(x);
        if(used == 0){
            setBlockBit(x, 1);
            break;
        }
    }
    uint32_t block = x+3+128;
    device->writeAll(block * BobFS::BLOCK_SIZE, zero_1024, 1024);
    
    return block;
}

///////////
// BobFS //
///////////

uint32_t BobFS::iroot =0;

BobFS::BobFS(Ide* device):device(device) {}

BobFS::~BobFS() {}

Node* BobFS::root(BobFS* fs) {
    Node* Root = new Node(fs,  BobFS::iroot);
    // Debug::printf("iroot:%d", iroot);
    return Root;
}

BobFS* BobFS::mount(Ide* device) {
    BobFS* fs = new BobFS(device);
    char readBuf[8];
    device->readAll(0, readBuf, 8);
    
    uint32_t root;
    device->readAll(8, &root, 4); 
    BobFS::iroot = root;
    return fs;
}

BobFS* BobFS::mkfs(Ide* device) {
    BobFS* fs = new BobFS(device);
    // superblock
    device->writeAll(0, zero_1024, BLOCK_SIZE);
    char* magicBuf = (char*)"BOBFS439";
    device->writeAll(0, magicBuf, 8);

    device->writeAll(START_OF_BLOCK_BITMAP, zero_1024, 1024);
    device->writeAll(START_OF_INODE_BITMAP, zero_1024, 1024);
    
    uint8_t aByte = 0;
    aByte |= 1;
    aByte |= 1<<1;
    aByte |= 1<<2;
    device->writeAll( START_OF_BLOCK_BITMAP, &aByte, 1);

    // make root node
    uint32_t inumber = 0;
    // setNodeBit(inumber, 1);
    aByte = 0;
    aByte |= 1;
    device->writeAll( START_OF_INODE_BITMAP, &aByte, 1);
    BobFS::iroot = inumber;
    device->writeAll(8, &BobFS::iroot, 4);
    
    // initNewNode(inumber, 1);
    uint16_t type=1;
    uint16_t nlinks=1;
    uint32_t size=0;
    uint32_t directBlock=0;
    uint32_t indirectBlock=0;
    uint32_t start = BobFS::START_OF_INODES + 16*inumber;
    device->writeAll(start, &type, 2);
    device->writeAll(start+2, &nlinks, 2);
    device->writeAll(start+4, &size, 4);
    device->writeAll(start+8, &directBlock, 4);
    device->writeAll(start+12, &indirectBlock, 4);
    
    return fs;
}
