
#ifndef LAT_QKNET_SERVER_H
#define LAT_QKNET_SERVER_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Stream.h"
#include "qknet/Acceptor.h"
#include "qknet/Connection.h"

class Session : public qknet::Connection {
public:
    Session(qkrtl::Poller& poller, qknet::ConnectionManager& sessions);
    virtual ~Session();

    virtual bool handleRead(qkrtl::Buffer& buffer);
    virtual bool handleWrited(qkrtl::Buffer& buffer);
    virtual bool handleOutput(int errCode = 0);
    virtual bool handleStop();
private:
    qkrtl::Stream inStream_;
    qknet::ConnectionManager& sessions_;
    qkrtl::Buffer outBuffer_;
    int64_t readSize_;
    int64_t writedSize_;
};

class Server : public qknet::Acceptor {
public:
    Server(qkrtl::Poller& poller);
    virtual ~Server();

    void final();

    virtual int handleAccept(int* handles, int size);
    virtual bool handleStop();
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::Poller& poller_;
    qknet::ConnectionManager sessions_;
};

#endif /**LAT_QKNET_SERVER_H*/
