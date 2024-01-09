
#pragma once

#include "PartlySuppressedCbStrategy.h"

namespace psi::comm {

template <typename... CbArgs>
PartlySuppressedCbStrategy<CbArgs...>::CbStrategy(const std::string &logPrefix)
    : BasicStrategy(asString(CbStrategyType::PartlySuppressedSync), logPrefix)
{
    logInfo("CbStrategy created");
}

template <typename... CbArgs>
PartlySuppressedCbStrategy<CbArgs...>::~CbStrategy()
{
    interrupt();

    logInfo("CbStrategy deleted");
}

template <typename... CbArgs>
void PartlySuppressedCbStrategy<CbArgs...>::interrupt()
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
void PartlySuppressedCbStrategy<CbArgs...>::processRequest(RequestFunc &&request, ResponseFunc &&response)
{
    if (m_isClosing) {
        return;
    }

    if (!m_queue.empty()) {
        m_queue.emplace(QueuedRequest {request, response});
        logInfo("queued request");
        return;
    }

    m_queue.emplace(QueuedRequest {request, response});
    processNext();
}

template <typename... CbArgs>
void PartlySuppressedCbStrategy<CbArgs...>::processNext()
{
    if (m_isClosing || m_queue.empty()) {
        return;
    }

    logInfo("process request");

    auto request = m_queue.front().first;
    request([this](CbArgs... values) {
        if (m_isClosing) {
            return;
        }
        logInfo("process response");

        auto response = m_queue.front().second;
        m_queue.pop();

        response(values...);

        while (m_queue.size() > 1) {
            logInfo("process next response");

            auto nextResponse = m_queue.front().second;
            m_queue.pop();

            nextResponse(values...);
        }

        processNext();
    });
}

} // namespace psi::comm