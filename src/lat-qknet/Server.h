
#ifndef LAT_QKNET_SERVER_H
#define LAT_QKNET_SERVER_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Stream.h"
#include "qknet/SocketAcceptor.h"
#include "qknet/SocketConnection.h"

class Session : public qknet::SocketConnectionHandler {
public:
    Session(qknet::SocketConnectionHandlerManager& sessions);
    virtual ~Session();
    virtual bool allocInBuffer(qkrtl::Buffer& buffer);
    virtual bool freeInBuffer(qkrtl::Buffer& buffer);
    virtual bool freeOutBuffer(qkrtl::Buffer& buffer);
    virtual bool handleInput(qkrtl::Buffer& buffer);
    virtual bool handleOutput(qkrtl::Buffer& buffer);
    virtual bool handleStop();
private:
    qkrtl::Stream inStream_;
    qknet::SocketConnectionHandlerManager& sessions_;
    int64_t readSize_;
    int64_t writedSize_;
};

class Server : public qknet::SocketAcceptHandler {
public:
    Server(qkrtl::Poller& poller);
    virtual ~Server();

    bool init(uint16_t port);
    void final();

    virtual bool handleAccepted(qknet::Socket& socket);
    virtual bool handleStop();
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::Poller& poller_;
    qknet::SocketAcceptHandle* acceptor_;
    qknet::SocketConnectionHandlerManager sessions_;
};

#endif /**LAT_QKNET_SERVER_H*/
