#include "psi/comm/CallHelper.h"

#include <iostream>
#include <set>

#include "AsyncStrategy.h"

int main()
{
    using namespace psi::examples;
    using namespace psi::comm::call_helper;

    // strategy class for processing async calls
    AsyncStrategy strategy;

    // final callback processes result callbacks of all requests
    auto finalCb = [](const std::vector<std::string> &results) {
        // result is sorted list of all responses
        std::set<std::string> sorted(results.begin(), results.end());
        for (const auto &str : sorted) {
            std::cout << str << std::endl;
        }
    };

    // generate 100 requests to be processed
    std::mutex logMutex;
    Requests<std::string> requests;
    for (size_t i = 0; i < 100; ++i) {
        requests.emplace_back(RequestPtr<std::string>(new Request<std::string>([&, i](ResponseCb<std::string> cb) {
            // emulate answers based on index
            const std::string answer = "result: " + std::to_string(i);

            {
                // synch log
                std::unique_lock<std::mutex> lock(logMutex);
                std::cout << "request[" << i << "] finished" << std::endl;
            }

            cb(answer);
        })));
    }

    // run requests asynchronouly
    runAllAsync<AsyncStrategy, std::string>(strategy, requests, finalCb);

    // wait for process finished before strategy is removed from a stack
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}