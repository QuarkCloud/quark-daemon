
#ifndef QKITC_CONNECTOR_H
#define QKITC_CONNECTOR_H 1

#include <time.h>
#include <string>

#include "qkrtl/Poller.h"
#include "qkitc/Compile.h"
#include "qkitc/Connection.h"

namespace qkitc {


class Connector : public Connection {
public:
    QKITCAPI Connector(qkrtl::Poller& poller);
    QKITCAPI virtual ~Connector();

    QKITCAPI bool open(const std::string& name);
};

}

#endif /**QKITC_CONNECTOR_H*/
