
#ifndef LAT_QKNET_CLIENT_H
#define LAT_QKNET_CLIENT_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qknet/SocketConnector.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/IoService.h"
#include "qkrtl/StateMonitor.h"
#include "qkrtl/PerfInfo.h"

class Client;
class ClientConnector : public qknet::ClientConnector {
public:
    ClientConnector(qkrtl::Poller& poller , Client * owner);
    virtual ~ClientConnector();
    virtual bool handleConnectSucceed(qknet::Socket& socket);
    virtual bool handleConnectFailed(int errCode);
    virtual bool handleConnectCompleted();
private:
    Client* owner_;
};

class Client : public qknet::SocketConnectionHandler  {
public:
    Client(qkrtl::Poller& poller);
    virtual ~Client();
    bool init(int maxTimes, int bufferSize);
    virtual void final();
    bool connect(const std::string& host, uint16_t port, int timeout);
    virtual bool allocInBuffer(qkrtl::Buffer& buffer);
    virtual bool freeInBuffer(qkrtl::Buffer& buffer);
    virtual bool freeOutBuffer(qkrtl::Buffer& buffer);
    virtual bool handleInput(qkrtl::Buffer& buffer);
    virtual bool handleOutput(qkrtl::Buffer& buffer);
    virtual bool handleStop();

    virtual bool handleConnectSucceed(qknet::Socket& socket);
    virtual bool handleConnectFailed(int errCode);

    bool waitForCompleted();

    inline int64_t elapse() const { return timeElapse_.elapse(); }

    static const int kConnectFailed = -1;
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::Poller& poller_;
    qkrtl::Buffer inBuffer_;
    qkrtl::Buffer outBuffer_;
    ClientConnector* connector_;
    qknet::SocketConnectionHandle* connection_;
    qkrtl::TimeElapse  timeElapse_;
    int maxTimes_;
    int bufferSize_;
    int64_t expectSize_;
    int64_t readSize_;
    int64_t writedSize_;
    qkrtl::StateMonitor states_;
};


#endif /**LAT_QKNET_CLIENT_H*/
