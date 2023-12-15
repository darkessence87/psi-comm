#include "psi/comm/CallHelper.h"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <vector>

class AsyncStrategy
{
public:
    AsyncStrategy()
    {
        m_threads.emplace_back(std::thread(std::bind(&AsyncStrategy::onThreadStart, this)));
        m_threads.emplace_back(std::thread(std::bind(&AsyncStrategy::onThreadStart, this)));
        m_threads.emplace_back(std::thread(std::bind(&AsyncStrategy::onThreadStart, this)));
        m_threads.emplace_back(std::thread(std::bind(&AsyncStrategy::onThreadStart, this)));
    }

    ~AsyncStrategy()
    {
        m_isActive = false;
        m_condition.notify_all();

        for (auto &t : m_threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    using Func = std::function<void()>;
    void asyncCall(Func &&fn)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(std::forward<Func>(fn));
        m_condition.notify_one();
    }

private:
    void onThreadStart()
    {
        while (m_isActive) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this]() { return !m_isActive || m_queue.size(); });

            if (m_queue.empty()) {
                continue;
            }

            auto fn = m_queue.front();
            m_queue.pop();

            lock.unlock();

            fn();
        }
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::queue<Func> m_queue;
    std::atomic<bool> m_isActive = true;
    std::vector<std::thread> m_threads;
};

int main()
{
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