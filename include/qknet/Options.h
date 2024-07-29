
#ifndef QKNET_OPTIONS_H
#define QKNET_OPTIONS_H 1

#include "qknet/Compile.h"

namespace qknet {

QKNETAPI bool OptionSetNodelay(int handle, bool value);
QKNETAPI bool OptionGetNodelay(int handle, bool& value);

QKNETAPI bool OptionSetNonBlocking(int handle, bool value);
QKNETAPI bool OptionGetNonBlocking(int handle, bool& value);

QKNETAPI bool OptionSetReuse(int handle, bool value);
QKNETAPI bool OptionGetReuse(int handle, bool& value);

QKNETAPI bool OptionSetKeepAlive(int handle, bool value);
QKNETAPI bool OptionGetKeepAlive(int handle, bool& value);

QKNETAPI bool OptionGetAvaibleSize(int handle, int& value);

QKNETAPI bool OptionSetSendBufferSize(int handle, int value);
QKNETAPI bool OptionGetSendBufferSize(int handle, int& value);

QKNETAPI bool OptionSetRecvBufferSize(int handle, int value);
QKNETAPI bool OptionGetRecvBufferSize(int handle, int& value);
    
QKNETAPI bool OptionGetSockType(int handle, int& value);

}
#endif /**QKNET_OPTIONS_H*/
