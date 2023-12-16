#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace psi::examples {

class ExampleStrategy
{
public:
    ExampleStrategy()
    {
        m_workThreads.emplace_back(std::thread(std::bind(&ExampleStrategy::onUpdate, this)));
        m_workThreads.emplace_back(std::thread(std::bind(&ExampleStrategy::onUpdate, this)));
    }

    ~ExampleStrategy()
    {
        m_isActive = false;
        m_cond.notify_all();

        for (auto &t : m_workThreads) {
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
    std::vector<std::thread> m_workThreads;
};

} // namespace psi::examples