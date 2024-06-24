
#include "qkinth/Object.h"
#include "qkrtl/Logger.h"

namespace qkinth {

Object::Object():
    freeTime_(0), peerHandle_(qkrtl::Object::kInvalidIdentifier) ,
    inEvent_(NULL), outEvent_(NULL)
{
    inBuffer_.init(kMaxBufferSize);
}
Object::~Object()
{
    inBuffer_.final();
}

int Object::read(char* buffer, int size)
{
    return inBuffer_.read(buffer, size);
}
void Object::inEvent(int bytes)
{
    doEvent(inEvent_, bytes);
}
void Object::outEvent(int bytes)
{
    doEvent(outEvent_, bytes);
}
void Object::doEvent(IoEvent* evt, int bytes)
{
    if (evt == NULL)
        return;

    if (bytes <= 0)
        evt->failure(bytes);
    else
        evt->success(bytes);
}
int Object::peerWrite(const char* buffer, int size)
{
    int writedSize = inBuffer_.write(buffer, size);
    inEvent(writedSize);
    return writedSize;
}
}
