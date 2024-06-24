
#include "qkrtl/Coder.h"

namespace qkrtl {


Decoder::Decoder()
{
    //
}
Decoder::~Decoder()
{
    //   
}
int Decoder::decode(const Buffer& buffer)
{
    return 0;
}
bool Decoder::begin()
{
    return true;
}
bool Decoder::end()
{
    return true;
}

Encoder::Encoder()
{
    //
}
Encoder::~Encoder()
{
    //
}
int Encoder::encode(Buffer& buffer) 
{
    return 0;
}
int Encoder::calcSize() const
{
    return 0;
}
bool Encoder::begin()
{
    return true;
}
bool Encoder::end()
{
    return true;
}

}
