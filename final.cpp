/*
 * Iyar Levi
 */

#include <iostream>
#include <vector>
#include <map>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "final.h"

FsFile::FsFile(int _block_size) { // constructor
// initialization
    file_size = 0;
    block_in_use = 0;
    block_size = _block_size;
    index_block = -1;
}
// getters and setters for FsFile

int FsFile::getBlock_in_use() {
    return block_in_use;
}
void FsFile::setBlock_in_use(int blockInUse) {
    block_in_use = blockInUse;
}
int FsFile::getIndex_block() {
    return index_block;
}
void FsFile::setIndex_block(int indexBlock) {
    index_block = indexBlock;
}
int FsFile::getfile_size() {
    return file_size;
}
void FsFile::setFile_size(int size) {
    file_size = size;
}
//---------------------------------------------------------------------------------------------------------------------

// getters and setters for FileDescriptor

bool FileDescriptor:: getInUse() {
    return inUse;
}

void FileDescriptor:: setInUse(bool in_Use) {
    inUse = in_Use;
}

FsFile *FileDescriptor::getFsFile() {
    return fs_file;
}
//---------------------------------------------------------------------------------------------------------------------

fsDisk::fsDisk() { // constructor
    sim_disk_fd = fopen(DISK_SIM_FILE , "r+");
    assert(sim_disk_fd);

    for (int i=0; i < DISK_SIZE ; i++) {
        int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
        ret_val = fwrite( "\0",  1, 1, sim_disk_fd);
        assert(ret_val == 1);
    }
    fflush(sim_disk_fd);
    is_formated = false;

    block_size = 0;
    available_blocks = 0;
}

fsDisk::~fsDisk() { // destructor that free all the allocated memory
    while (!MainDir.empty()){ // remove the MainDir data
        DelFile(MainDir[0] -> getFileName());
    }
    MainDir.clear(); // empty the MainDir vector

    // delete everything else
    delete[] OpenFileDescriptors;
    delete[] BitVector;

    fclose(sim_disk_fd); // close the file
}

void fsDisk::listAll() { // func that print the data in the disk
    int i;
    for (i = 0; i < (int) MainDir.size(); i++) {
        cout << "index: " << i << ": FileName: " << MainDir[i] -> getFileName() << " , getInUse: " << MainDir[i] -> getInUse() <<
             endl;
    }
    char bufy;
    cout << "Disk content: '";
    for (i = 0; i < DISK_SIZE; i++) {
        cout << "(";
        int ret_val = fseek(sim_disk_fd, i, SEEK_SET); // put the currser in the right location
        ret_val = fread( &bufy, 1, 1, sim_disk_fd); // read from the file
        cout << bufy;
        cout << ")";
    }
    cout << "'" << endl;
}

void fsDisk::fsFormat(int blockSize) { // func that format the file
    // initialize the data
    block_size = blockSize;
    BitVectorSize = DISK_SIZE / blockSize;
    available_blocks = this -> BitVectorSize;
    max_size = blockSize * blockSize;

    if (is_formated) { // if the disc is already formatted
        while (!MainDir.empty()) { // if the mainDir isn't empty, clear everything in the mainDir
            DelFile(MainDir[0]->getFileName());
        }
        MainDir.clear();

        delete[] BitVector;
        delete[] OpenFileDescriptors;
    }
    // set up the new format
    BitVector = new int[BitVectorSize]; // dynamic allocated memory
    OpenFileDescriptors = new FileDescriptor* [BitVectorSize]; // dynamic allocated memory

    for (int i = 0; i < BitVectorSize; i++) {
        BitVector[i] = 0; // the "memory" is not in use
        OpenFileDescriptors[i] = nullptr; // not in use, use nullptr because its more "flexible"
    }
    is_formated = true; // after we formatted the disc we want to update that
}

int fsDisk::CreateFile(string fileName) { // func that creating new file
    if (!is_formated || MainDir.size() >= BitVectorSize) { // if you can't create more files
        return -1;
    }
    else{ // if you can add the file
        int fd_number=-1;
        FsFile* fileToAdd = new FsFile(block_size); // create the new file
        FileDescriptor * file_fd = new FileDescriptor(fileName, fileToAdd);

        for(int i=0; i<BitVectorSize; i++){ // find the first open space
            if(OpenFileDescriptors[i] == nullptr) {
                fd_number=i;
                break;
            }
        }
        if (fd_number == -1){
            return -1;
        }

        OpenFileDescriptors[fd_number] = file_fd; // add the tmp_fd to the openFD "list"
        addIndexBlock(fileToAdd); //set index block in the new file
        MainDir.emplace_back(file_fd); // add the tmp_fd at the END of the mainDir vector
        return fd_number;
    }
}

int fsDisk::OpenFile(string fileName) { // func that open the file
    if (is_formated) {
        for (int i = 0; i < (int)MainDir.size(); ++i) { // find the file by his name
            if (MainDir[i] -> getFileName() == fileName) {
                if (MainDir[i] -> getInUse()) { // if the file is already open, return -1
                    return -1;
                }
                else { // if the file is close, open the file
                    int j = 0;
                    int fd_number;
                    while (j < BitVectorSize && OpenFileDescriptors[j++]); // find the first open space
                    if (j <= BitVectorSize){
                        fd_number = j-1;
                    }
                    else{ // there is no open space
                        fd_number = -1;
                    }
                    OpenFileDescriptors[fd_number] = MainDir[i];
                    MainDir[i] -> setInUse(true); // now the file is open, so he is in use
                    return fd_number;
                }
            }
        }
    }
    return -1;
}

string fsDisk::CloseFile(int fd) { // func that close the file
    if (is_formated && OpenFileDescriptors[fd]) { // check if the file is open and formatted
        string file_to_close = OpenFileDescriptors[fd] -> getFileName();
        int where;
        for (int i = 0; i < (int) MainDir.size(); i++) { // search the file in mainDir
            if (MainDir[i] -> getFileName() == file_to_close)
                where = i;
        }
        if (where == -1){ // file not found
            return "-1";
        }
        MainDir[where] -> setInUse(false); // the file isn't in use anymore
        OpenFileDescriptors[fd] = nullptr; // take out from the open files
        return file_to_close;
    }
    return "-1";
}

int fsDisk::WriteToFile(int fd, char * buf, int len) { // func that write data to the disk
    if( len == 0) // stopping condition
        return 1;
    FileDescriptor* file = OpenFileDescriptors[fd];
    if(file == nullptr && is_formated)
        return -1;
    FsFile* fsFile = file->getFsFile();
    if(max_size - fsFile->getfile_size() < len) // max file size is smaller/equal then wanted
        return -1;

    int indexBlock = fsFile -> getIndex_block();
    int blockUsed = fsFile -> getBlock_in_use(); // how many block are in use in this file
    if(blockUsed == 0){ // the file hasn't been written before
        int block_to_use = (len % block_size == 0) ? len / block_size : (len / block_size) + 1;
        if(block_to_use > available_blocks){ // number of block available greater then wanted
            return -1;
        }
        for(int i=0; i < block_to_use; i++){ // write the data to the block
            int blockToWriteTo = getFreeBlock();
            fseek(sim_disk_fd,blockToWriteTo * block_size,SEEK_SET);
            int charsToWrite = (len > block_size) ? block_size : len;
            fwrite(buf,charsToWrite,1,sim_disk_fd);
            len -= charsToWrite;
            buf += charsToWrite; // leave in the buf only what we didn't write yet
            char blockToWriteToIndex='0';
            decToBinary(blockToWriteTo, blockToWriteToIndex); // use of decToBinary function
            fseek(sim_disk_fd, indexBlock * block_size + blockUsed, SEEK_SET);
            fwrite(&blockToWriteToIndex,1,1,sim_disk_fd); // write the index block number
            blockUsed++;
            fsFile->setBlock_in_use(blockUsed); // update the block in use for this file
            fsFile->setFile_size(fsFile->getfile_size()+charsToWrite); // update the size of this file
        }
        return 1;
    }
    // if the file has been written to before
    if(fsFile->getfile_size() % block_size == 0){ // no free allocated space left
        int block_to_use = (len % block_size == 0) ? len / block_size : (len / block_size) + 1;
        if(block_to_use > available_blocks){ // number of block available greater then wanted
            return -1;
        }
        for(int i=0; i < block_to_use; i++){
            int blockToWriteTo = getFreeBlock();
            fseek(sim_disk_fd,blockToWriteTo * block_size,SEEK_SET);//CHECK IF blockToWriteToIndex is needed here
            int charsToWrite = (len > block_size) ? block_size : len;
            fwrite(buf,charsToWrite,1,sim_disk_fd);
            len -= charsToWrite;
            buf += charsToWrite;
            char blockToWriteToIndex='0';
            decToBinary(blockToWriteTo, blockToWriteToIndex);
            fseek(sim_disk_fd, indexBlock * block_size + blockUsed, SEEK_SET);
            fwrite(&blockToWriteToIndex,1,1,sim_disk_fd);
            blockUsed++;
            fsFile->setBlock_in_use(blockUsed);
            fsFile->setFile_size(fsFile->getfile_size()+charsToWrite);
        }
        return 1;
    }
    // if there is space in the last block
    int nextCharinLastBlock = fsFile->getfile_size() - (fsFile->getBlock_in_use() - 1) * block_size;
    fseek(sim_disk_fd, indexBlock * block_size + blockUsed - 1, SEEK_SET);
    char lastBlockInUsed;
    fread( &lastBlockInUsed,1,1,sim_disk_fd); // find the index of the last block
    int lastBlockInInt = (int)lastBlockInUsed - 48;
    fseek(sim_disk_fd, lastBlockInInt * block_size + nextCharinLastBlock,SEEK_SET);
    int charToWrite = (len > (block_size - nextCharinLastBlock)) ? block_size - nextCharinLastBlock : len;
    fwrite(buf,charToWrite,1,sim_disk_fd);
    len -= charToWrite;
    buf += charToWrite;
    fsFile->setFile_size(fsFile->getfile_size()+charToWrite); // update the size of this file
    return WriteToFile(fd, buf, len); // recursive
}

int fsDisk::ReadFromFile(int fd, char * buf, int len) { // func that read from the disk
    int sizeToRead;
    int blocksToRead;
    FileDescriptor* file = OpenFileDescriptors[fd];
    if(file == nullptr && is_formated)
        return -1;
    int available_size = file -> getFsFile() -> getfile_size();
    if (is_formated && len <= available_size) { // you can read from the file
        FsFile* fsFile = file -> getFsFile();
        if(len % block_size == 0){ // how many blocks to read
            blocksToRead = len / block_size;
        }
        else{
            blocksToRead = (len / block_size) +1;
        }
        if(block_size < len){ // do you need to read the whole block or just part of it
            sizeToRead = block_size;
        }
        else{
            sizeToRead = len;
        }
        char currBlock;
        int locInDisk = block_size * (fsFile -> getIndex_block()); // from where to read
        for (int i = 0; i < blocksToRead; i++) {
            int ret_val1 = fseek(sim_disk_fd, locInDisk + i, SEEK_SET); // the location of the block
            if (fread( & currBlock, 1, 1, sim_disk_fd) < 1)
                return -1;
            int ret_val2 = fseek(sim_disk_fd, block_size * (int) currBlock, SEEK_SET);
            if (fread(buf, sizeToRead, 1, sim_disk_fd) < 1)
                return -1;

            buf += sizeToRead;
            len -= sizeToRead;
            if(block_size < len) { // recheck how much to read
                sizeToRead = block_size;
            }
            else {
                sizeToRead = len;
            }
        }
        strncpy(buf, "\0", 1); // put \0 at the end to "close" the string
        return len;
    }
    return -1;
}

int fsDisk::DelFile(string FileName) { // func that delete the file and the data from the disk
    if (is_formated) {
        int fileNameLocation;
        for (int i = 0; i < (int) MainDir.size(); i++) { // search the file in MainDir by name
            if (MainDir[i] -> getFileName() == FileName)
                fileNameLocation = i;
        }
        if (fileNameLocation != -1) { // if we found the file
            FileDescriptor * file = MainDir[fileNameLocation];
            FsFile * fsFile = file -> getFsFile(); // get the target file
            int indexOfTheBlock = fsFile -> getIndex_block();

            for(int i=0; i < fsFile->getBlock_in_use(); i++){
                fseek(sim_disk_fd,indexOfTheBlock * block_size + i, SEEK_SET);
                char dataBlockIndex;
                fread(&dataBlockIndex, 1,1,sim_disk_fd);
                releaseBlock((int)dataBlockIndex); // release all the data that in the block
            }
            releaseBlock(fsFile->getIndex_block());
            int fd_deleted = 0;
            for(int i=0; i < BitVectorSize ; i++){
                if(OpenFileDescriptors[i]->getFileName() == FileName) { // remove the file from the OpenFileDescriptors
                    fd_deleted = i;
                    i = BitVectorSize;
                }
            }
            // deleting all the data connected to the file
            delete fsFile;
            delete MainDir[fileNameLocation];
            MainDir.erase(MainDir.begin() + fileNameLocation);
            OpenFileDescriptors[fd_deleted] = nullptr;
            return  fd_deleted;
        }
    }
    return -1;
}

void fsDisk::decToBinary(int n, char & c) {
    // array to store binary number
    int binaryNum[8];
    // counter for binary array
    int i = 0;
    while (n > 0) {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }
    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--) {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}

int fsDisk::addIndexBlock(FsFile *fsFile) { // func that set the available block as our block - ADDED BY ME
    if(available_blocks == 0){
        return  -1;
    }
    int index = getFreeBlock();
    if(index == -1){
        return -1;
    }
    fsFile ->setIndex_block(index);
    return index;
}

int fsDisk::getFreeBlock() { // func that get us the first available block - ADDED BY ME
    for (int i = 0; i < BitVectorSize; i++) {
        if (BitVector[i] == 0) {
            BitVector[i] = 1; //The block is now in use
            available_blocks--; // one more block is now unavailable
            return i;
        }
    }
    return -1;
}

int fsDisk::releaseBlock(int blockIndex) { // func that release the block and his data - ADDED BY ME
    if(blockIndex < 0 || blockIndex >= BitVectorSize)
        return -1;
    fseek(sim_disk_fd,block_size * blockIndex, SEEK_SET);
    for( int i = 0; i < block_size ; i++)
        fwrite("\0",1,1,sim_disk_fd);
    BitVector[blockIndex] = 0;
    available_blocks++;
    return 1;
}
