
#ifndef QKNET_ACCEPTOR_H
#define QKNET_ACCEPTOR_H 1

#include <time.h>
#include <set>
#include <list>
#include "qknet/Compile.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Buffer.h"
#include "qknet/IoEvent.h"
#include "qknet/IoHandle.h"

namespace qknet{


class Acceptor : public IoHandle{
public:
    QKNETAPI Acceptor(qkrtl::Poller& poller);
    QKNETAPI virtual ~Acceptor();

    QKNETAPI bool init(uint16_t port);
    QKNETAPI void final();

    QKNETAPI virtual void doAsynDestroy();
    QKNETAPI virtual void close();

    //和Poller的关联切断完成
    QKNETAPI virtual bool handleStop();

    QKNETAPI virtual bool handleInput(int errCode = 0);
    QKNETAPI virtual int handleAccept(int * handles , int size);
    QKNETAPI virtual void notifyRead(int errCode);
private:
    std::mutex guard_;
    bool finaled_;
    TcpSocketAcceptor acceptor_;
};

}
#endif /**QKNET_ACCEPTOR_H*/
