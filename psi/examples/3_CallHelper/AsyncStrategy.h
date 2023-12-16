#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace psi::examples {

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

} // namespace psi::examples