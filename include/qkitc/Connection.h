
#ifndef QKITC_CONNECTION_H
#define QKITC_CONNECTION_H 1

#include <time.h>
#include <string>
#include <mutex>

#include "qkrtl/Buffer.h"
#include "qkrtl/IoEvent.h"
#include "qkrtl/Poller.h"
#include "qkitc/Compile.h"
#include "qkitc/IoHandle.h"

namespace qkitc {

class Connection : public IoHandle {
public:
    QKITCAPI Connection(qkrtl::Poller& poller);
    QKITCAPI virtual ~Connection();

    QKITCAPI virtual void close();

    QKITCAPI virtual bool handleStart();
    QKITCAPI virtual bool handleStop();

    QKITCAPI virtual bool handleInput(int errCode = 0);
    QKITCAPI virtual bool startInput();

    QKITCAPI virtual bool handleOutput(int errCode = 0);
    QKITCAPI virtual bool startOutput();

    QKITCAPI virtual bool valid() const;
protected:
    qkrtl::Poller& poller_;
    qkrtl::InEvent reader_;
    qkrtl::OutEvent writer_;

    QKITCAPI virtual void notifyRead(int errCode);
    QKITCAPI virtual void notifyWrite(int bytes , int errCode);
};


}

#endif /**QKITC_CONNECTION_H*/
