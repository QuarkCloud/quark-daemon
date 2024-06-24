
#ifndef QKINTH_FILE_IO_H
#define QKINTH_FILE_IO_H 1

#include <string>
#include "qkinth/Compile.h"
#include "qkinth/IoEvent.h"

namespace qkinth {

QKINTHAPI int FileCreate(const std::string& name);
QKINTHAPI int FileOpen(const std::string& name);
QKINTHAPI void FileClose(int handle);
QKINTHAPI int FileRead(int handle , char * buffer , int size);
QKINTHAPI int FileWrite(int handle, char* buffer, int size);
QKINTHAPI bool FileValid(int handle);
QKINTHAPI bool FileSetIoEvent(int handle , IoEvent * in , IoEvent * out);

}

#endif /**QKINTH_FILE_IO_H*/
