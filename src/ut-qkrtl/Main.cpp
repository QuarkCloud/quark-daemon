
#include <stdio.h>
#include "qkrtl/TestKits.h"

int main(int argc, char* argv[])
{
    qkrtl::TestCaseManager::singleton().run();
    return 0;
}

