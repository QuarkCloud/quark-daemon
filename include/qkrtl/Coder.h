
#ifndef QKRTL_CODER_H
#define QKRTL_CODER_H 1

#include <stdint.h>
#include <string.h>
#include "qkrtl/Compile.h"
#include "qkrtl/Buffer.h"

namespace qkrtl {

class QKRTLAPI Decoder {
public:
    Decoder();
    virtual ~Decoder();
    virtual int decode(const Buffer& buffer);
    virtual bool begin();
    virtual bool end();
};

class QKRTLAPI Encoder {
public:
    Encoder();
    virtual ~Encoder();

    virtual int encode(Buffer& buffer) ;
    virtual int calcSize() const;
    virtual bool begin();
    virtual bool end();
};

}

#endif /**QKRTL_CODER_H*/
