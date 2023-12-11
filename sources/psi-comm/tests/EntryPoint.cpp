#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#define LOG_INFO_STATIC(x) std::cout << x << std::endl;

using namespace ::testing;

int main(int argc, char **argv)
{
    int result = 0;
    auto runMain = [&]() {
        LOG_INFO_STATIC("Start tests main");

        InitGoogleTest(&argc, argv);

        result = RUN_ALL_TESTS();

        LOG_INFO_STATIC("Exit tests main");
    };

    runMain();

#ifdef _WIN32
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();
#endif

    return result;
}
