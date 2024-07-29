
#include "qkrtl/TestKits.h"
#include "qkrtl/PowerTwo.h"

bool testCeilingNextPowerOfTwo()
{
    int result1 = qkrtl::CeilingNextPowerOfTwo(1);
    TESTCMP(IsEqual(result1, 1));

    int result2 = qkrtl::CeilingNextPowerOfTwo(2);
    TESTCMP(IsEqual(result2, 2));

    int result3 = qkrtl::CeilingNextPowerOfTwo(3);
    TESTCMP(IsEqual(result3, 4));

    return true;
}
bool testLogOfTwo()
{
    TESTCMP(IsEqual(qkrtl::LogOfTwo(1), 0));
    TESTCMP(IsEqual(qkrtl::LogOfTwo(2), 1));
    TESTCMP(IsEqual(qkrtl::LogOfTwo(3), 1));
    TESTCMP(IsEqual(qkrtl::LogOfTwo(4), 2));
    return true;
}

//TESTCASE(testCeilingNextPowerOfTwo);
//TESTCASE(testLogOfTwo);