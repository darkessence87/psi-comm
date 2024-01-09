
#include "SuppressedCbStrategy.h"

namespace psi::comm {

template <typename... CbArgs>
SuppressedCbStrategy<CbArgs...>::CbStrategy(const std::string &logPrefix)
    : BasicStrategy(asString(CbStrategyType::SuppressedSync), logPrefix)
{
    logInfo("CbStrategy created");
}

template <typename... CbArgs>
SuppressedCbStrategy<CbArgs...>::~CbStrategy()
{
    interrupt();

    logInfo("CbStrategy deleted");
}

template <typename... CbArgs>
void SuppressedCbStrategy<CbArgs...>::interrupt()
{
    m_isClosing = true;

    while (!m_queue.empty()) {
        m_mutex.lock();
        auto response = m_queue.front().second;
        m_queue.pop();
        m_mutex.unlock();

        logInfo("send failed response on processor interruption");
        std::tuple<CbArgs...> values;
        VariadicCaller<CbArgs...>::invoke(response, values);
    }
}

template <typename... CbArgs>
void SuppressedCbStrategy<CbArgs...>::processRequest(RequestFunc request, ResponseFunc response)
{
    if (m_isClosing) {
        return;
    }

    if (!m_queue.empty()) {
        m_queue.emplace(QueuedRequest {request, response});
        logInfo("queued request");
        return;
    }

    logInfo("process request");

    m_queue.emplace(QueuedRequest {request, response});
    request([this](CbArgs... values) {
        while (!m_isClosing && !m_queue.empty()) {
            logInfo("process response");

            auto resp = m_queue.front().second;
            m_queue.pop();

            resp(values...);
        }
    });
}

} // namespace psi::comm