
#ifndef QKRTL_TEST_KITS_H
#define QKRTL_TEST_KITS_H 1

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

#include "qkrtl/Compile.h"

typedef bool (*LPFN_TestCase)();

namespace qkrtl {


class TestCase {
public:
    QKRTLAPI TestCase(const TestCase& testCase);
    QKRTLAPI TestCase(const std::string& name, LPFN_TestCase testCase);
    QKRTLAPI virtual ~TestCase();
    QKRTLAPI virtual std::string execute();
    inline int index() const { return index_; }
    inline bool result() const { return result_; }
    inline const std::string& name() const { return name_; }
private:
    friend class TestCaseManager;
    int index_;
    bool result_;
    std::string name_;
    LPFN_TestCase lpfnTestCase_;
};

class TestCaseManager {
public:
    QKRTLAPI TestCaseManager();
    QKRTLAPI virtual ~TestCaseManager();
    QKRTLAPI bool add(const std::string& name, LPFN_TestCase testCase);
    QKRTLAPI bool add(const TestCase& testCase);
    QKRTLAPI void run();
    QKRTLAPI static TestCaseManager& singleton();
private:
    std::vector<TestCase> testCases_;
};

class QKRTLAPI TestCaseRegier {
public:
    TestCaseRegier(const std::string& name, LPFN_TestCase testCase);
};

QKRTLAPI std::string PickTestInfo(const char* expr, const char* function, const char* fileName, int line);

}


template<typename T>
bool IsEqual(const T& src, const T& dst)
{
    return (src == dst);
}
template<typename T>
bool IsGreater(const T& src, const T& dst)
{
    return (src > dst);
}
template<typename T>
bool IsGEqual(const T& src, const T& dst)
{
    return (src >= dst);
}
template<typename T>
bool IsLEqual(const T& src, const T& dst)
{
    return (src <= dst);
}

#define TESTCMP(x) \
do{\
    if((x) == false) \
        throw std::exception(qkrtl::PickTestInfo(#x , __FUNCTION__ , __FILE__ , __LINE__).c_str()); \
}while(false)

#define TESTCASE(x) static qkrtl::TestCaseRegier __TestCaseRegier##x##_(#x , x)

#endif /**QKRTL_TEST_KITS_H*/
