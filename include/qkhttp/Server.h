
#ifndef QKHTTP_SERVER_H
#define QKHTTP_SERVER_H 1

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include "qkrtl/Buffer.h"
#include "qkrtl/Stream.h"
#include "qkrtl/Poller.h"
#include "qknet/SocketAcceptor.h"
#include "qknet/SocketConnection.h"

#include "qkhttp/Compile.h"
#include "qkhttp/String.h"
#include "qkhttp/Coder.h"
#include "qkhttp/Header.h"

namespace qkhttp {

class QKHTTPAPI UriHandler {
public:
    virtual bool handle(const Request& request, Response& response);
};

class Session : public qknet::SocketConnectionHandler {
public:
    QKHTTPAPI Session(qknet::SocketConnectionHandlerManager& sessions);
    QKHTTPAPI virtual ~Session();
    QKHTTPAPI virtual bool allocInBuffer(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool freeInBuffer(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool freeOutBuffer(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool handleInput(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool handleOutput(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool handleStop();

    /**
        ʵ�ֿͻ��˷�������Ľ���������Ҫ��URL���ã������򿴾���Ӧ�õ�����
    */
    QKHTTPAPI bool handleRequest(const Request& request);
    QKHTTPAPI bool resetHandler(UriHandler * handler);
private:
    qknet::SocketConnectionHandlerManager& sessions_;
    qkrtl::Buffer inBuffer_;
    qkrtl::Stream outStream_;
    UriHandler* handler_;
};

class Server : public qknet::SocketAcceptHandler {
public:
    QKHTTPAPI Server(qkrtl::Poller& poller);
    QKHTTPAPI virtual ~Server();

    QKHTTPAPI bool init(uint16_t port);
    QKHTTPAPI void final();

    QKHTTPAPI virtual bool handleAccepted(qknet::Socket& socket);
    QKHTTPAPI virtual bool handleStop();

    QKHTTPAPI bool resetHandler(UriHandler* handler);
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::Poller& poller_;
    qknet::SocketAcceptHandle* acceptor_;
    qknet::SocketConnectionHandlerManager sessions_;
    UriHandler* handler_;
};

}


#endif /**QKHTTP_SERVER_H*/
