
#ifndef QKNET_CONNECTOR_H
#define QKNET_CONNECTOR_H 1

#include "qknet/Compile.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Timer.h"
#include "qknet/IoHandle.h"
#include "qknet/IoEvent.h"
#include "qknet/Connection.h"

namespace qknet{

/**
    �ڳ������Ӻ���Ҫȷ�����ض�ʱ��󣬱����ܹ����ӳɹ���������Ϊʧ�ܡ�
*/
class ConnectTimeout : public qkrtl::TimerTask {
public:
    QKNETAPI ConnectTimeout(Connection& connection);
    QKNETAPI virtual ~ConnectTimeout();
    QKNETAPI virtual void execute();
private:
    Connection& connection_;
};

class Connector : public Connection {
public:
    QKNETAPI Connector(qkrtl::Poller& poller);
    QKNETAPI virtual ~Connector();

    //timeoutΪ���뼶
    QKNETAPI bool connect(const std::string& host, uint16_t port , int timeout);

    QKNETAPI virtual void final();

    QKNETAPI virtual void notifyOpen(int errCode);
    static const int kTimeout = 1000000;
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::IoStatus ioStatus_;
    ConnectTimeout timeout_;
    TcpSocketConnector connector_;

    QKNETAPI virtual void doAsynDestroy();
};


}
#endif /**QKNET_CONNECTOR_H*/
