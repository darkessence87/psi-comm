
#include <iostream>

#include "psi/comm/call_strategy/cb/AsyncCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/FullySyncCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/SuppressedCbStrategy.hpp"

using namespace psi::comm;

int main()
{
    /**
     * @brief Usage example.
     * General requirements.
     * 1) We need to provide total cost of some product, which consists of costs of all components, on demand by clients.
     * 2) Provided data has to be consistent.
     * 
     * Constraints. 
     * 1) Cost might be very dynamic (like traffic on roads, weather, distance to destination, or currency price).
     * 2) Cost of each component is provided by external system, so that communication is async.
     * 
     * Problem. Another (or same) client may request total cost before we calculated the previous one.
     * 
     * Possible solutions.
     * 1) [VERY BAD] Block current thread and wait for all components' costs for current client. Obviously, bottleneck.
     * 
     * 2) [BAD] Group component's request into client's context and run everything asyncronously.
     * Still not good, because component's callbacks may come in different order, so that final results might be sent also in different order.
     * If the client is the same for both requests, data is provided inconsistently (older data is sent later).
     * 
     * 3) [MEDIUM] Group component's request into client's context and run context via queue.
     * For some cases this approach might be good as all requirements are met, and current thread is not blocked.
     * Possible implementation:
     *  a) state machine might be very complex or will make code complexity overhead
     *  b) psi::comm::FullySyncCbStrategy is very simple solution for this
     * 
     * 4) [GOOD] Group clients and request components' data only once. On collecting full data response to all clients.
     * Possible implementation:
     *  a) state machine might be very complex or will make code complexity overhead
     *  b) psi::comm::SuppressedCbStrategy is very simple solution for this
     */
    using ResultCb = std::function<void(int)>;
    std::deque<ResultCb> m_queue;
    auto getComponentsData = [&](int reqId, ResultCb cb) {
        std::cout << "getComponentsData reqId: " << reqId << std::endl;
        m_queue.emplace_back(cb);
    };

    auto getTotal = [=](auto &strategy, int reqId, ResultCb cb) {
        std::cout << "getTotal reqId: " << reqId << std::endl;
        strategy.processRequest([=](auto cb) { getComponentsData(reqId, cb); }, cb);
    };
    const int N = 3;

    /// bad approach
    {
        AsyncCbStrategy<int> st;
        for (int i = 1; i <= N; ++i) {
            getTotal(st, i, [=](auto total) {
                std::cout << "[RESULTS] reqId: " << i << ", total: " << total << std::endl;
            });
        }
        // emulate different order of callbacks
        int i = 0;
        while (!m_queue.empty()) {
            auto fn = m_queue.back();
            m_queue.pop_back();
            fn(++i);
        }
        std::cout << std::endl;
    }

    /// medium approach
    {
        FullySyncCbStrategy<int> st;
        for (int i = 1; i <= N; ++i) {
            getTotal(st, i, [=](auto total) {
                std::cout << "[RESULTS] reqId: " << i << ", total: " << total << std::endl;
            });
        }
        // emulate different order of callbacks
        int i = 0;
        while (!m_queue.empty()) {
            auto fn = m_queue.back();
            m_queue.pop_back();
            fn(++i);
        }
        std::cout << std::endl;
    }

    /// good approach
    {
        SuppressedCbStrategy<int> st;
        for (int i = 1; i <= N; ++i) {
            getTotal(st, i, [=](auto total) {
                std::cout << "[RESULTS] reqId: " << i << ", total: " << total << std::endl;
            });
        }
        // emulate different order of callbacks
        int i = 0;
        while (!m_queue.empty()) {
            auto fn = m_queue.back();
            m_queue.pop_back();
            fn(++i);
        }
        std::cout << std::endl;
    }
}