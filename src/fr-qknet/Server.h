
#ifndef FR_QKNET_SERVER_H
#define FR_QKNET_SERVER_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/Poller.h"
#include "qkrtl/PerfInfo.h"
#include "qkrtl/Stream.h"
#include "qknet/Acceptor.h"
#include "qknet/Connection.h"

class Session : public qknet::Connection {
public:
    Session(qkrtl::Poller& poller , qknet::ConnectionManager& sessions);
    virtual ~Session();

    virtual bool handleInput(int errCode = 0);
    virtual bool handleOutput(int errCode = 0);
    virtual bool handleError(int errCode);
    virtual bool handleStop();
private:
    qkrtl::Stream inStream_;
    qknet::ConnectionManager& sessions_;
};

class Server : public qknet::Acceptor {
public:
    Server(qkrtl::Poller& poller);
    virtual ~Server();

    bool init(uint16_t port);
    void final();

    virtual int handleAccept(int* handles, int size);
    virtual bool handleStop();
private:
    std::mutex guard_;
    bool finaled_;
    qknet::ConnectionManager sessions_;
};

#endif /**FR_QKNET_SERVER_H*/
