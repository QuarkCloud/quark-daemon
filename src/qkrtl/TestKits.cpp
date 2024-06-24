#include "qkrtl/TestKits.h"


namespace qkrtl {

TestCase::TestCase(const TestCase& testCase)
    :index_(-1), result_(false), lpfnTestCase_(NULL)
{
    index_ = testCase.index_;
    result_ = testCase.result_;
    name_ = testCase.name_;
    lpfnTestCase_ = testCase.lpfnTestCase_;
}
TestCase::TestCase(const std::string& name, LPFN_TestCase testCase)
    :index_(-1), result_(false), name_(name), lpfnTestCase_(testCase)
{
    //
}
TestCase::~TestCase()
{
    //
}

std::string TestCase::execute()
{
    result_ = false;
    char message[1024] = { '\0' };
    int offset = ::sprintf(message, "[%04d] TestCase[%s] ", index_, name_.c_str());
    try {
        if (lpfnTestCase_ == NULL)
        {
            ::sprintf(message + offset, "not implemented");
            return message;
        }
        result_ = lpfnTestCase_();
        ::sprintf(message + offset, "result[%s]", (result_ ? "true" : "false"));
        return message;
    }
    catch (const std::exception& e)
    {
        result_ = false;
        ::sprintf(message + offset, "failed, errmsg=%s", e.what());
    }

    return message;
}

TestCaseManager::TestCaseManager()
{
    //
}
TestCaseManager::~TestCaseManager()
{
    //
}
bool TestCaseManager::add(const std::string& name, LPFN_TestCase testCase)
{
    return add(TestCase(name, testCase));
}
bool TestCaseManager::add(const TestCase& testCase)
{
    size_t lastIndex = testCases_.size();
    testCases_.push_back(testCase);
    testCases_[lastIndex].index_ = (int)lastIndex;
    return true;
}
void TestCaseManager::run()
{
    int successCounter = 0, failedCounter = 0;
    std::vector<std::string> messages;

    for (size_t tidx = 0; tidx < testCases_.size(); ++tidx)
    {
        TestCase& testCase = testCases_[tidx];
        std::string message = testCase.execute();
        if (testCase.result() == true)
            ++successCounter;
        else
            ++failedCounter;
        messages.push_back(message);
    }

    ::printf("TestCase run completed , total[%d] success[%d] failed[%d]\n",
        (int)testCases_.size(), successCounter, failedCounter);

    for (size_t midx = 0; midx < messages.size(); ++midx)
    {
        ::printf("%s\n", messages[midx].c_str());
    }
}
TestCaseManager& TestCaseManager::singleton()
{
    static TestCaseManager __TestCaseManagerSingleton__;
    return __TestCaseManagerSingleton__;
}
TestCaseRegier::TestCaseRegier(const std::string& name, LPFN_TestCase testCase)
{
    TestCaseManager::singleton().add(name, testCase);
}


std::string PickTestInfo(const char* expr, const char* function, const char* fileName, int line)
{
    char str[1024] = { '\0' };

    const char* start = fileName + ::strlen(fileName);
    while (start != fileName)
    {
        char ch = *start;
        if (ch == '/' || ch == '\\')
        {
            start++;
            break;
        }
        --start;
    }

    ::sprintf(str, "expr[%s] function[%s] file[%s:%d]",
        expr, function, start, line);
    return str;
}

}

