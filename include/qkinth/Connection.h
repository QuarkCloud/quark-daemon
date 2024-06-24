
#ifndef QKINTH_CONNECTION_H
#define QKINTH_CONNECTION_H 1

#include <time.h>
#include <string>
#include <mutex>

#include "qkrtl/IoHandle.h"
#include "qkrtl/Poller.h"
#include "qkinth/Compile.h"
#include "qkinth/IoEvent.h"

namespace qkinth {

class Connection;
class Reader : public IoEvent {
public:
    QKINTHAPI Reader(Connection* owner);
    QKINTHAPI virtual ~Reader();
    QKINTHAPI virtual void execute();
private:
    Connection* owner_;
};

class Writer : public IoEvent {
public:
    QKINTHAPI Writer(Connection* owner);
    QKINTHAPI virtual ~Writer();
    QKINTHAPI virtual void execute();
private:
    Connection* owner_;
};

class Connection : public qkrtl::IoHandle {
public:
    QKINTHAPI Connection(qkrtl::Poller& poller);
    QKINTHAPI virtual ~Connection();

    QKINTHAPI virtual bool create(const std::string& name);
    QKINTHAPI virtual bool open(const std::string& name);
    QKINTHAPI virtual void close();

    QKINTHAPI virtual bool handleStart();
    QKINTHAPI virtual bool handleStop();

    QKINTHAPI virtual bool handleInput(int errCode = 0);
    QKINTHAPI virtual bool startInput();

    QKINTHAPI virtual bool handleOutput(int errCode = 0);
    QKINTHAPI virtual bool startOutput();

    QKINTHAPI virtual const HANDLE getHandle() const;
    QKINTHAPI virtual bool valid() const;

    inline int handle() const { return handle_; }
protected:
    qkrtl::Poller& poller_;
    Reader reader_;
    Writer writer_;
    int handle_;
};


}

#endif /**QKINTH_CONNECTION_H*/
