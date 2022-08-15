# disk-management

disk management
authored by Iyar Levi

------------DESCRIPTION------------------------------------------------------------------------------


The program simulate the disk management. 


------------files--------------------------------------------------------------------------------


main.cpp

final.h

final.cpp

NOTE - you will need to create a file called "DISK_SIM_FILE.txt" - this file will simulate 

the disk in this program.
 

------------functions--------------------------------------------------------------------------------

constractor - fsDisk() 


destructor - ~fsDisk() 


print - listAll() - 1


format - fsFormat(int blockSize) - 2


create files - CreateFile(string fileName) - 3


open files - OpenFile(string fileName) - 4


close files - CloseFile(int fd) - 5


writing to file - WriteToFile(int fd, char * buf, int len) - 6


reading from file - ReadFromFile(int fd, char * buf, int len) - 7


deleting file - DelFile(string FileName) - 8


------------how to use--------------------------------------------------------------------------------

every function has a number between 1 and 8.
when the main is running, choose wich function to activate by pressing 
the number that relate to the function.
