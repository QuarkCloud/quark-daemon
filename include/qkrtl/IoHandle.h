
#ifndef QKRTL_IO_HANDLE_H
#define QKRTL_IO_HANDLE_H 1

#include <mutex>
#include <deque>
#include "qkrtl/Compile.h"
#include "qkrtl/EventPoller.h"

namespace qkrtl {

class IoHandle : public EventNode{
public:
    QKRTLAPI IoHandle();
    QKRTLAPI virtual ~IoHandle();

    QKRTLAPI virtual void close();

    QKRTLAPI virtual bool handleStart();
    QKRTLAPI virtual bool handleStop();

    QKRTLAPI virtual bool handleInput(int errCode = 0);
    QKRTLAPI virtual bool startInput();

    QKRTLAPI virtual bool handleOutput(int errCode = 0);
    QKRTLAPI virtual bool startOutput();

    QKRTLAPI virtual const HANDLE getHandle() const;
    QKRTLAPI virtual bool valid() const;
};

class IoHandler : public EventNode{
public:
    QKRTLAPI IoHandler();
    QKRTLAPI virtual ~IoHandler();

    QKRTLAPI virtual void close();

    QKRTLAPI virtual bool start();
    QKRTLAPI virtual void stop();

    QKRTLAPI virtual bool handleStart();
    QKRTLAPI virtual bool handleStop();
    QKRTLAPI virtual bool handle(int errCode = 0);

    QKRTLAPI virtual const HANDLE getHandle() const;
    QKRTLAPI virtual bool valid() const;
};

}
#endif /**QKRTL_IO_HANDLE_H*/
