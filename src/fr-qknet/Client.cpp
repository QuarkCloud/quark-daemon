
#include "Client.h"
#include "qkrtl/Logger.h"

Client::Client(qkrtl::Poller& poller):qknet::ClientConnector(poller)
{
    //
}
Client::~Client()
{
    //
}
bool Client::handleConnectCompleted()
{
    waiter_.addCounter();
    return true;
}
bool Client::waitForCompleted()
{
    bool result = waiter_.waitForCompleted(1, -1);
    LOGINFO("Client[%p] wait completed , result[%s]" , this , result?"True":"False");
    return result;
}



