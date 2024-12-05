/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
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
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */


/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    // Allocate memory for the inode list and free-block list
    inodes = new Inode[MAX_INODES];
    free_blocks = new unsigned char[SimpleDisk::BLOCK_SIZE];

    Console::puts("File system initialized.\n");
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    // Write the inode list to the disk
    disk->write(0, reinterpret_cast<unsigned char*>(inodes));

    // Write the free-block list to the disk
    disk->write(1, free_blocks);

    // Free allocated memory
    delete[] inodes;
    delete[] free_blocks;

    Console::puts("File system unmounted.\n");
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");
    /* Here you read the inode list and the free list into memory */
    disk = _disk;

    // Read the inode list from the disk
    disk->read(0, reinterpret_cast<unsigned char*>(inodes));

    // Read the free-block list from the disk
    disk->read(1, free_blocks);

    Console::puts("File system mounted.\n");
    return true;
    assert(false);
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    // Allocate temporary memory for the inode list and free-block list
    Inode* temp_inodes = new Inode[MAX_INODES];
    unsigned char* temp_free_blocks = new unsigned char[SimpleDisk::BLOCK_SIZE];

    // Initialize the inode list with IDs set to -1
    for (unsigned int i = 0; i < MAX_INODES; i++) {
        temp_inodes[i].id = -1;
    }

    // Initialize the free-block list
    memset(temp_free_blocks, 0, SimpleDisk::BLOCK_SIZE);

    // Mark the INODES block and FREELIST block as used
    temp_free_blocks[0] = 1;
    temp_free_blocks[1] = 1;

    // Write the inode list to the disk
    _disk->write(0, reinterpret_cast<unsigned char*>(temp_inodes));

    // Write the free-block list to the disk
    _disk->write(1, temp_free_blocks);

    // Free allocated memory
    delete[] temp_inodes;
    delete[] temp_free_blocks;

    Console::puts("Disk formatted.\n");
    return true;
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    // Iterate through the inode list to find the file
    for (unsigned int i = 0; i < MAX_INODES; i++) {
        if (inodes[i].getId() == _file_id) {
            return &inodes[i];
        }
    }

    // File not found
    return nullptr;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    if (LookupFile(_file_id) != nullptr) {
        Console::puts("Error: File already exists\n");
        return false;
    }

    // Find a free inode
    for (unsigned int i = 0; i < MAX_INODES; i++) {
        if (inodes[i].getId() == -1) {
            // Find a free block
            for (unsigned int j = 2; j < SimpleDisk::BLOCK_SIZE; j++) { // Start from block 2 to avoid INODES and FREELIST blocks
                if (free_blocks[j] == 0) {
                    // Allocate the block
                    free_blocks[j] = 1;

                    // Initialize the inode
                    inodes[i] = Inode(_file_id, this);
                    inodes[i].setBlockNumber(j);

                    // Write the inode list to the disk
                    disk->write(0, reinterpret_cast<unsigned char*>(inodes));

                    // Write the free-block list to the disk
                    disk->write(1, free_blocks);


                    Console::puts("File created.\n");
                    return true;
                }
            }

            Console::puts("Error: No free blocks available\n");
            return false;
        }
    }

    Console::puts("Error: No free inodes available\n");
    return false;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
    // Check if the file exists
    Inode* inode = LookupFile(_file_id);
    if (inode == nullptr) {
        Console::puts("Error: File not found\n");
        return false;
    }

    // Free the block occupied by the file
    free_blocks[inode->getBlockNumber()] = 0;

    // Invalidate the inode
    inode->id = -1;
    inode->size = 0;
    inode->block_number = 0;
    inode->fs = nullptr;

    Console::puts("File deleted.\n");
    return true;
}
