
#ifndef QKRTL_POWER_TWO_H
#define QKRTL_POWER_TWO_H 1

#include "qkrtl/Compile.h"

namespace qkrtl {

/**
    返回不小于x的，最小2的幂。
*/
QKRTLAPI int CeilingNextPowerOfTwo(int x);

QKRTLAPI bool IsPowerOfTwo(int x);
/**
    LogOfTwo(1) = 0 ;
    LogOfTwo(2) = 1 ;
    LogOfTwo(3) = 1 ;   //这个需要特别注意
    LogOfTwo(4) = 2 ;

    int result = LogOfTwo(x) ;
    x <= 2 ^ result
*/
QKRTLAPI int LogOfTwo(int x);

QKRTLAPI int AlignSize(int x , int align);

}
#endif /**QKRTL_POWER_TWO_H*/
