#include "psi/test/psi_test.h"

#ifdef PSI_LOGGER
#include "psi/logger/Logger.h"
#else
#include <iostream>
#include <sstream>
#define LOG_INFO_STATIC(x)                                                                                             \
    do {                                                                                                               \
        std::ostringstream os;                                                                                         \
        os << x;                                                                                                       \
        std::cout << os.str() << std::endl;                                                                            \
    } while (0)
#endif

extern void register_all_tests();

int main(int argc, char *argv[])
{
    LOG_INFO_STATIC("Start tests main");

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-container"
    auto args = std::span<char *> {argv, static_cast<size_t>(argc)};
#pragma clang diagnostic pop
    auto opts = psi::test::TestLib::parse_args(args);

    psi::test::TestLib::init();
    register_all_tests();
    int result = psi::test::TestLib::run(opts.filter);

    psi::test::TestLib::destroy();
    LOG_INFO_STATIC("Exit tests main");

    return result;
}
