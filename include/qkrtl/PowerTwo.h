
#ifndef QKRTL_POWER_TWO_H
#define QKRTL_POWER_TWO_H 1

#include "qkrtl/Compile.h"

namespace qkrtl {

/**
    ���ز�С��x�ģ���С2���ݡ�
*/
QKRTLAPI int CeilingNextPowerOfTwo(int x);

QKRTLAPI bool IsPowerOfTwo(int x);
/**
    LogOfTwo(1) = 0 ;
    LogOfTwo(2) = 1 ;
    LogOfTwo(3) = 1 ;   //�����Ҫ�ر�ע��
    LogOfTwo(4) = 2 ;

    int result = LogOfTwo(x) ;
    x <= 2 ^ result
*/
QKRTLAPI int LogOfTwo(int x);

QKRTLAPI int AlignSize(int x , int align);

}
#endif /**QKRTL_POWER_TWO_H*/
