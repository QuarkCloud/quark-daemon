
#ifndef LAT_QKITC_SERVER_H
#define LAT_QKITC_SERVER_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Stream.h"
#include "qkitc/Connection.h"
#include "qkitc/Acceptor.h"

/**
    简单的echo，用于测试延迟
*/
class Session : public qkitc::Connection {
public:
    Session(qkrtl::Poller& poller);
    virtual ~Session();
    virtual bool handleInput(int errCode = 0);
    virtual bool handleOutput(int errCode = 0);
private:
    std::mutex guard_;
};

class Server : public qkitc::Acceptor{
public:
    Server(qkrtl::Poller& poller);
    virtual ~Server();
    void final();

    virtual bool handleError(int errCode);
    virtual bool handleAccept(int* handles, int size);
private:
    std::mutex guard_;
    bool finaled_;
    std::list<Session*> sessions_;
};

#endif /**LAT_QKITC_SERVER_H*/
