#ifndef _BOBFS_H_
#define _BOBFS_H_

#include "ide.h"

class BobFS;


/* In-memory representation of an i-node */
class Node {
   BobFS* myFS;
   Ide* device;
   uint32_t inumber;
   // uint16_t type;
   // uint16_t nlinks;
   // uint32_t size;
   // uint32_t directBlock;
   // uint32_t indirectBlock;
   uint32_t totalBlocks;
   // uint32_t* blockIdx;

public:
    static constexpr uint32_t NODES_PER_BLOCK = 64;
    static constexpr uint32_t NODE_SIZE = 16;
    static constexpr uint16_t DIR_TYPE = 1;
    static constexpr uint16_t FILE_TYPE = 2;

    /* Create a Node object representing the given i-node
       in the given file system */
    Node(BobFS* fs, uint32_t inumber);
    
    /* node type */
    uint16_t getType(void);

    /* number of links to this node */
    uint16_t getLinks(void);

    /* size of the data represented by this node */
    uint32_t getSize(void);

    void initNewNode(const uint32_t inumber, const uint32_t type);
    uint32_t getInumber(void);
    void addLink(void);

    bool getBlockBit(uint32_t bitoffset);
    bool getNodeBit(uint32_t bitoffset);
    void setBlockBit(uint32_t bitoffset, bool bit);
    void setNodeBit(uint32_t bitoffset, bool bit);
   //  void cleanBlock(uint32_t blocktoWrite);
    uint32_t findEmptyBlock();
    uint32_t getDirect();
    uint32_t getIndirect();
    uint32_t getTotalBlocks();

    void writeBlock(uint32_t blocktoWrite, const void* buffer);
    void readBlock(uint32_t blocktoRead, void* buffer);
    /* read up to "n" bytes and store them in the given "buffer"
       starting at the given "offset" in the file

       returns: number of bytes actually read
                x = 0      ==> end of file
                x < 0      ==> error
                0 > x <= n ==> number of bytes actually read
     */ 
    int32_t read(uint32_t offset, void* buffer, uint32_t n);


    /* like read but promises to read as many bytes as it can */
    int32_t readAll(uint32_t offset, void* buffer, uint32_t n);

    /* writes up to "n" bytes from the given "buffer"
       starting at the given "offset" in the file

       returns: number of bytes actually written
                x = 0      ==> end of file
                x < 0      ==> error
                0 > x <= n ==> number of bytes actually written
     */ 
    int32_t write(uint32_t offset, const void* buffer, uint32_t n);

    /* like write but promises to write as many bytes as it can */
    int32_t writeAll(uint32_t offset, const void* buffer, uint32_t n);

    /* If the current node is a directory, create an entry
       with the given information and return a pointer to the
       Node representing the new entry

       returns null if the current Node is not a directory
     */
    Node* newNode(const char* name, uint32_t type);

    /* calls newNode to create a file */
    Node* newFile(const char* name);

    /* calls newNode to create a directory */
    Node* newDirectory(const char* name);

    /* if the current node is a directory, returns a pointer
       the entry with the given name */
    Node* findNode(const char* name);

    bool isFile(void);
    bool isDirectory(void);

    /* Creates a new link to the given node in this directory */
    /* does nothing of the current node is not a directory */
    void linkNode(const char* name, Node* file);

    void dump(const char* name);

    static Node* get(BobFS* fs, uint32_t index) {
        Node* n = new Node(fs,index);
        return n;
    }
};


/* In-memory representation of a BobFS file system */
class BobFS {

public:
    static constexpr uint32_t BLOCK_SIZE = 1024;
    static constexpr uint32_t START_OF_BLOCK_BITMAP = 1024;
    static constexpr uint32_t START_OF_INODE_BITMAP = 2 * 1024;
    static constexpr uint32_t START_OF_INODES = 3 * 1024;
    static constexpr uint32_t START_OF_BLOCK = 3 * 1024 + 128 * 1024;
    Ide* device;
    static uint32_t iroot;
   //  static uint8_t blockBitmap[8*BLOCK_SIZE];
   //  static uint8_t inodeBitmap[8*BLOCK_SIZE];
    
    BobFS(Ide* device);
    virtual ~BobFS();

    /* make a new BobFS file system on the given device */
    static BobFS* mkfs(Ide* device);

    /* mount an existing BobFS file system from the given device */
    static BobFS* mount(Ide* device);

    /* Return a pointer to the root node of the given file system */
    static Node* root(BobFS* fs);

    
    
};

#endif
