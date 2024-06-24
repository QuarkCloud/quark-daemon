
#ifndef LAT_QKINTH_SERVER_H
#define LAT_QKINTH_SERVER_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Stream.h"
#include "qkinth/Connection.h"

/**
    �򵥵�echo�����ڲ����ӳ�
*/
class Server : public qkinth::Connection{
public:
    Server(qkrtl::Poller& poller);
    virtual ~Server();

    void final();
    virtual bool handleInput(int errCode = 0);
    virtual bool handleOutput(int errCode = 0);
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::Buffer buffer_;
};

#endif /**LAT_QKINTH_SERVER_H*/
