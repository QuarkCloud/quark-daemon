
#ifndef QKITC_ACCEPTOR_H
#define QKITC_ACCEPTOR_H 1

#include "qkrtl/Poller.h"
#include "qkrtl/IoEvent.h"
#include "qkitc/Compile.h"
#include "qkitc/IoHandle.h"

namespace qkitc {

class Acceptor : public IoHandle {
public:
    QKITCAPI Acceptor(qkrtl::Poller& poller);
    QKITCAPI virtual ~Acceptor();

    QKITCAPI virtual bool create(const std::string& name);
    QKITCAPI virtual void close();

    QKITCAPI virtual bool handleStart();
    QKITCAPI virtual bool handleStop();

    QKITCAPI virtual bool handleInput(int errCode = 0);
    QKITCAPI virtual bool startInput();
    QKITCAPI virtual bool valid() const;

    QKITCAPI virtual bool handleError(int errCode);
    QKITCAPI virtual bool handleAccept(int * handles , int size);
protected:
    qkrtl::Poller& poller_;
    qkrtl::InEvent acceptor_;
    QKITCAPI virtual void notifyRead(int errCode);
};

}

#endif /**QKITC_ACCEPTOR_H*/
