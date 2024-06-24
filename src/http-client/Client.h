
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H 1

#include "qkhttp/Client.h"

class Client : public qkhttp::ClientRequest {
public:
    Client();
    virtual ~Client();

    virtual bool onRely(const qkhttp::Response& response);
    bool connect(const std::string& host, uint16_t port, int timeout);
};


#endif /**HTTP_CLIENT_H*/
