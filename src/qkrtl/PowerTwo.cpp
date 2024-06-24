
#include "qkrtl/PowerTwo.h"

namespace qkrtl {

int CeilingNextPowerOfTwo(int x)
{
    int result = 1;
    while (result < x)
    {
        result <<= 1;
    }

    return result;
}

bool IsPowerOfTwo(int x)
{
    return x > 0 && (x & (x - 1)) == 0;
}

int LogOfTwo(int x)
{
    int log = 0;
    while ((x >>= 1) != 0)
    {
        ++log;
    }
    return log;
}

int AlignSize(int x, int align)
{
    align = CeilingNextPowerOfTwo(align) ;
    int mask = align - 1;

    if ((x & mask) == 0)
        return x;

    return (x & (~mask)) + align;
}

}
