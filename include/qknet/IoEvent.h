
#ifndef QKNET_IO_EVENT_H
#define QKNET_IO_EVENT_H 1

#include <mutex>
#include "qkrtl/IoEvent.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/IoHandle.h"

#include "qknet/Compile.h"

namespace qknet{

/**
    read �¼���Ҫ����ϵͳ�ײ������ģ��ǵ����¼���
    ��û�����ݿ��Ա���ȡʱ����һ�������ڵײ㡣
*/
class TcpSocketReader : public qkrtl::InEvent {
public:
    QKNETAPI TcpSocketReader(qkrtl::IoHandle& handle);
    QKNETAPI virtual ~TcpSocketReader();

    QKNETAPI bool read(qkrtl::Buffer& buffer);
    QKNETAPI virtual void close();
    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();
private:
    mutable std::mutex guard_;
};
class TcpSocketWriter : public qkrtl::OutEvent {
public:
    QKNETAPI TcpSocketWriter(qkrtl::IoHandle& handle);
    QKNETAPI virtual ~TcpSocketWriter();

    /**
        ���˿���д��ʱ���ڴ�ҳ�ᱻϵͳ���������Ա��뼰ʱ�˳���
        ���������0ʱ����������д������û���㹻�ռ�
    */
    QKNETAPI bool write(qkrtl::Buffer& buffer);
    QKNETAPI virtual void close();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();

private:
    mutable std::mutex guard_;
};

class TcpSocketAcceptor : public qkrtl::InEvent {
public:
    QKNETAPI TcpSocketAcceptor(qkrtl::IoHandle& handle);
    QKNETAPI virtual ~TcpSocketAcceptor();
    QKNETAPI virtual void close();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();
};

/**
    ����Ҫȡ������࣬����Ҫ�и�eventNode���󶨵�poller
*/
class TcpSocketConnector : public qkrtl::InEvent {
public:
    QKNETAPI TcpSocketConnector(qkrtl::IoHandle& handle);
    QKNETAPI virtual ~TcpSocketConnector();
    QKNETAPI virtual void close();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();
};

}
#endif /**QKNET_IO_EVENT_H*/
