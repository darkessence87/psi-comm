
#pragma once

#include "FullySyncCbStrategy.h"

namespace psi::comm {

template <typename... CbArgs>
FullySyncCbStrategy<CbArgs...>::CbStrategy(const std::string &logPrefix)
    : BasicStrategy(asString(CbStrategyType::FullySync), logPrefix)
{
    logInfo("CbStrategy created");
}

template <typename... CbArgs>
FullySyncCbStrategy<CbArgs...>::~CbStrategy()
{
    interruptImmediately();

    logInfo("CbStrategy deleted");
}

template <typename... CbArgs>
void FullySyncCbStrategy<CbArgs...>::interrupt()
{
    m_isClosing = true;

    while (!m_interruptImmediately && !m_queue.empty()) {
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
void FullySyncCbStrategy<CbArgs...>::interruptImmediately()
{
    m_interruptImmediately = true;
    interrupt();
}

template <typename... CbArgs>
void FullySyncCbStrategy<CbArgs...>::processRequest(RequestFunc request, ResponseFunc response)
{
    if (m_isClosing) {
        return;
    }

    if (!m_queue.empty()) {
        m_mutex.lock();
        m_queue.emplace(QueuedRequest {request, response});
        logInfo("queued request");
        m_mutex.unlock();
        return;
    }

    m_mutex.lock();
    m_queue.emplace(QueuedRequest {request, response});
    m_mutex.unlock();
    processNext();
}

template <typename... CbArgs>
void FullySyncCbStrategy<CbArgs...>::processNext()
{
    if (m_queue.empty()) {
        return;
    }

    logInfo("process request");

    auto request = m_queue.front().first;
    request([this](CbArgs... values) {
        if (m_isClosing || m_queue.empty()) {
            return;
        }

        logInfo("process response");

        m_mutex.lock();
        auto response = m_queue.front().second;
        m_queue.pop();
        const bool needProcessNext = !m_queue.empty();
        m_mutex.unlock();

        response(values...);

        if (needProcessNext) {
            processNext();
        }
    });
}

} // namespace psi::comm