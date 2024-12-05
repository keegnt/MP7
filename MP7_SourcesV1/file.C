/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem * _fs, int _id) : fs(_fs), current_position(0) {
    inode = fs->LookupFile(_id);
    if (inode) {
        // Load the block from disk into the cache
        fs->readBlock(inode->getBlockNumber(), block_cache);
    } else {
        // Handle error: file not found
        Console::puts("Error: File not found\n");
        assert(false);
    }
}

File::~File() {
    Console::puts("Closing file.\n");
    // Write any cached data to disk
    fs->writeBlock(inode->getBlockNumber(), block_cache);

    // Update the inode list on disk
    fs->updateInode(inode);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");

    unsigned int bytes_to_read = _n;
    if (current_position + _n > inode->getSize()) {
        bytes_to_read = inode->getSize() - current_position;
    }

    memcpy(_buf, block_cache + current_position, bytes_to_read);
    current_position += bytes_to_read;

    return bytes_to_read;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");

    unsigned int bytes_to_write = _n;
    if (current_position + _n > SimpleDisk::BLOCK_SIZE) {
        bytes_to_write = SimpleDisk::BLOCK_SIZE - current_position;
    }

    memcpy(block_cache + current_position, _buf, bytes_to_write);
    current_position += bytes_to_write;

    if (current_position > inode->getSize()) {
        inode->setSize(current_position);
    }

    fs->writeBlock(inode->getBlockNumber(), block_cache);

    return bytes_to_write;
}

void File::Reset() {
    Console::puts("resetting file\n");
    current_position = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    return current_position >= inode->getSize();
}

