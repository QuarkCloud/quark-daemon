
#include "Client.h"
#include "qkrtl/Logger.h"

Client::Client()
{
    //
}
Client::~Client()
{
    //
}
bool Client::onRely(const qkhttp::Response& response)
{
    return false;
}
bool Client::connect(const std::string& host, uint16_t port, int timeout)
{
    return false;
}


