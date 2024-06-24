
#ifndef QKHTTP_CLIENT_H
#define QKHTTP_CLIENT_H 1

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/Poller.h"
#include "qknet/SocketConnector.h"
#include "qkhttp/Compile.h"
#include "qkhttp/Header.h"
#include "qkhttp/Coder.h"

namespace qkhttp {

class ClientRequest {
public:
    QKHTTPAPI ClientRequest();
    QKHTTPAPI virtual ~ClientRequest();

    QKHTTPAPI bool init(const Request& request);
    QKHTTPAPI bool init(qkrtl::Buffer& request);
    QKHTTPAPI void final();
    QKHTTPAPI bool request(qkrtl::Buffer& buffer);
    QKHTTPAPI bool reply(qkrtl::Buffer& buffer);
    QKHTTPAPI bool replyEnd();

    QKHTTPAPI virtual bool onRely(const Response& response);
private:
    bool finaled_;
    qkrtl::Buffer outBuffer_;
    qkrtl::Stream inStream_;
};

class Client;
class ClientConnector : public qknet::ClientConnector {
public:
    QKHTTPAPI ClientConnector(qkrtl::Poller& poller, Client* owner);
    QKHTTPAPI virtual ~ClientConnector();
    QKHTTPAPI virtual bool handleConnectSucceed(qknet::Socket& socket);
    QKHTTPAPI virtual bool handleConnectFailed(int errCode);
    QKHTTPAPI virtual bool handleConnectCompleted();
private:
    Client* owner_;
};

class Client : public qknet::SocketConnectionHandler {
public:
    QKHTTPAPI Client(qkrtl::Poller& poller);
    QKHTTPAPI virtual ~Client();

    QKHTTPAPI bool init(ClientRequest* request);
    QKHTTPAPI virtual void final();
    QKHTTPAPI bool connect(const std::string& host, uint16_t port, int timeout);
    QKHTTPAPI virtual bool allocInBuffer(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool freeInBuffer(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool freeOutBuffer(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool handleInput(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool handleOutput(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool handleStop();

    QKHTTPAPI virtual bool handleConnectSucceed(qknet::Socket& socket);
    QKHTTPAPI virtual bool handleConnectFailed(int errCode);

    static const int kConnectFailed = -1;
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::Poller& poller_;
    ClientConnector* connector_;
    qknet::SocketConnectionHandle* connection_;
    ClientRequest* request_;
};
}


#endif /**QKHTTP_CLIENT_H*/
