/*
 * Iyar Levi
 */

#ifndef FINAL_PROJECT_FINAL_H
#define FINAL_PROJECT_FINAL_H

#include <vector>

using namespace std;
#define DISK_SIZE 256
#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

//

// ============================================================================

class FsFile {
    int file_size;
    int block_in_use;
    int index_block;
    int block_size;

public:
    FsFile(int _block_size);

    // add getters and setters

    int getBlock_in_use();
    void setBlock_in_use(int blockInUse);

    int getIndex_block();
    void setIndex_block(int indexBlock);

    int getfile_size();
    void setFile_size(int size);

};

// ============================================================================

class FileDescriptor {
    string file_name;
    FsFile* fs_file;
    bool inUse;

public:

    FileDescriptor(string FileName, FsFile* fsi) {
        file_name = FileName;
        fs_file = fsi;
        inUse = true;
    }

    // getters and setters
    string getFileName() {
        return file_name;
    }

    bool getInUse();
    void setInUse(bool in_Use);
    FsFile* getFsFile();
};

// ============================================================================

class fsDisk {
    FILE *sim_disk_fd;
    bool is_formated;
    int block_size;
    int available_blocks; // how much free blocks available
    int max_size;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //             or not. (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.

    int BitVectorSize;
    int *BitVector;

    // (5) MainDir --
    // Structure that links the file name to its FsFile

    vector <FileDescriptor*> MainDir; // vector of FileDescriptor* that contain the file name and its fsFile

    // (6) OpenFileDescriptors --
    //  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.

    FileDescriptor ** OpenFileDescriptors; // contain only the OPEN files

public:
    // ------------------------------------------------------------------------
    fsDisk();
    // ------------------------------------------------------------------------
    ~fsDisk();
    // ------------------------------------------------------------------------
    void listAll();
    // ------------------------------------------------------------------------
    void fsFormat( int blockSize =4 );  // check the "=4"
    // ------------------------------------------------------------------------
    int CreateFile(string fileName);
    // ------------------------------------------------------------------------
    int OpenFile(string fileName);
    // ------------------------------------------------------------------------
    string CloseFile(int fd);
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len );
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len );
    // ------------------------------------------------------------------------
    int DelFile( string FileName );
    // ------------------------------------------------------------------------
    void decToBinary(int n, char &c);

    // my functions
    int addIndexBlock(FsFile* fsFile);
    int getFreeBlock();
    int releaseBlock(int blockIndex);


};

#endif //FINAL_PROJECT_FINAL_H