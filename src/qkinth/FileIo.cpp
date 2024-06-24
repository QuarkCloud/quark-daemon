
#include "qkinth/FileIo.h"
#include "qkinth/FileSystem.h"

namespace qkinth {

int FileCreate(const std::string& name)
{
    return FileSystem::singleton().create(name);
}
int FileOpen(const std::string& name)
{
    return FileSystem::singleton().open(name);
}
void FileClose(int handle)
{
    FileSystem::singleton().close(handle);
}
int FileRead(int handle, char* buffer, int size)
{
    return FileSystem::singleton().read(handle, buffer, size);
}
int FileWrite(int handle, char* buffer, int size)
{
    return FileSystem::singleton().write(handle, buffer, size);
}
bool FileValid(int handle)
{
    return FileSystem::singleton().valid(handle);
}
bool FileSetIoEvent(int handle, IoEvent* in, IoEvent* out)
{
    Object* obj = FileSystem::singleton().find(handle);
    if (obj == NULL)
        return false;

    obj->inEvent(in);
    obj->outEvent(out);
    return true;
}

}

