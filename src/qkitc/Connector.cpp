
#include "qkitc/Connector.h"
#include "qkrtl/FileSystem.h"

namespace qkitc {

Connector::Connector(qkrtl::Poller& poller):Connection(poller)
{

}
Connector::~Connector()
{
    //
}
bool Connector::open(const std::string& name)
{
    int index = FsOpen(name, this);
    setHandle(index);
    return valid();
}
}