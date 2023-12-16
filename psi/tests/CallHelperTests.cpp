#include "TestHelper.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "psi/comm/CallHelper.h"
#undef private

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

using namespace ::testing;
using namespace psi::comm::call_helper;
using namespace psi::test;

TEST(CallHelperTests, runAll)
{
    Requests<bool> requests;
    using CbType = std::function<void(bool)>;

    const size_t requestsNumber = 100;
    MockedFn<CbType> finalCb;
    const int expectedNumberOfSuccess = 50;
    const int expectedSum = requestsNumber * 10;
    int value = 0;

    for (size_t i = 0; i < requestsNumber; ++i) {
        requests.emplace_back(RequestPtr<bool>(new Request<bool>([&, i](ResponseCb<bool> cb) {
            value += 10;
            cb(i % 2 == 0);
        })));
    }

    EXPECT_CALL(finalCb, f(true));
    runAll<bool>(requests, [requestsNumber, expectedNumberOfSuccess, cb = finalCb.fn()](std::vector<bool> results) {
        ASSERT_EQ(results.size(), requestsNumber);

        int numberOfSuccess = 0;
        for (auto r : results) {
            numberOfSuccess += r ? 1 : 0;
        }
        EXPECT_EQ(numberOfSuccess, expectedNumberOfSuccess);

        cb(true);
    });

    EXPECT_EQ(expectedSum, value);
}

TEST(CallHelperTests, runAllAsync)
{
    class TestStrategy
    {
    public:
        TestStrategy()
        {
            m_workThread = std::thread(std::bind(&TestStrategy::onUpdate, this));
        }

        ~TestStrategy()
        {
            m_isActive = false;
            m_cond.notify_all();

            if (m_workThread.joinable()) {
                m_workThread.join();
            }
        }

        using Func = std::function<void()>;
        void asyncCall(Func &&fn)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.emplace(std::forward<Func>(fn));
            m_cond.notify_one();
        }

    private:
        void onUpdate()
        {
            while (m_isActive) {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cond.wait(lock, [this]() { return !m_isActive || m_queue.size(); });

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
        std::condition_variable m_cond;
        std::queue<Func> m_queue;
        std::atomic<bool> m_isActive = true;
        std::thread m_workThread;
    };
    TestStrategy strategy;

    Requests<bool> requests;
    using CbType = std::function<void(bool)>;

    const size_t requestsNumber = 100;
    MockedFn<CbType> finalCb;
    const int expectedNumberOfSuccess = 50;
    const int expectedSum = requestsNumber * 10;
    std::atomic<int> value = 0;

    for (size_t i = 0; i < requestsNumber; ++i) {
        requests.emplace_back(RequestPtr<bool>(new Request<bool>([&, i](ResponseCb<bool> cb) {
            value += 10;
            cb(i % 2 == 0);
        })));
    }

    EXPECT_CALL(finalCb, f(true));
    runAllAsync<TestStrategy, bool>(strategy,
                                    requests,
                                    [requestsNumber, expectedNumberOfSuccess, cb = finalCb.fn()](std::vector<bool> results) {
                                        ASSERT_EQ(results.size(), requestsNumber);

                                        int numberOfSuccess = 0;
                                        for (auto r : results) {
                                            numberOfSuccess += r ? 1 : 0;
                                        }
                                        EXPECT_EQ(numberOfSuccess, expectedNumberOfSuccess);

                                        cb(true);
                                    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(expectedSum, value);
}
