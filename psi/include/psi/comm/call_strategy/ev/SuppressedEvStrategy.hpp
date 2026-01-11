
#pragma once

#include "SuppressedEvStrategy.h"

namespace psi::comm {

template <typename... EvArgs, typename... CbArgs>
EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>>::EvStrategy(const std::string &logPrefix)
    : BasicStrategy(asString(EvStrategyType::SuppressedSync), logPrefix)
{
    logInfo("EvStrategy created");
}

template <typename... EvArgs, typename... CbArgs>
EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>>::~EvStrategy()
{
    m_isClosing = true;

    while (!m_requestQueue.empty()) {
        m_mutex.lock();
        auto ev = std::get<2>(m_requestQueue.front());
        m_requestQueue.pop_front();
        m_mutex.unlock();

        logInfo("send failed response on processor deletion");
        std::tuple<EvArgs...> values;
        VariadicCaller<EvArgs...>::invoke(ev, values);
    }

    logInfo("EvStrategy deleted");
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>>::processRequest(
    RequestFunc request,
    ResponseFunc response,
    EventFunc onEvent)
{
    if (m_isClosing) {
        return;
    }

    if (!m_requestQueue.empty() || m_isPaused) {
        m_requestQueue.emplace_back(QueuedRequest {request, response, onEvent});
        logInfo("queued request");
        return;
    }

    logInfo("process request");

    m_requestQueue.emplace_back(QueuedRequest {request, response, onEvent});
    processNext();
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>>::processEvent(EvArgs... args)
{
    if (m_isClosing) {
        return;
    }

    if (!m_isWaitingForEvent) {
        logInfo("Not waiting for event. Ignore.");
        return;
    }

    m_isWaitingForEvent = false;

    logInfo("process event");

    sendResults(args...);
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>>::pauseRequest()
{
    if (m_isClosing) {
        return;
    }

    m_isPaused = true;

    if (!m_requestQueue.empty()) {
        auto requestCopy = m_requestQueue.front();
        std::get<2>(requestCopy) = [this](EvArgs...) { logInfo("ignored invalidated response"); };
        m_requestQueue.emplace_front(requestCopy);
    }
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>>::continueRequest()
{
    if (m_isClosing) {
        return;
    }

    m_isPaused = false;

    if (!m_requestQueue.empty() && !m_isWaitingForEvent) {
        processNext();
    }
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>>::processNext()
{
    if (m_isClosing) {
        return;
    }

    if (m_requestQueue.empty()) {
        return;
    }

    auto request = std::get<0>(m_requestQueue.front());
    request([this](CbArgs... values) -> bool {
        if (m_isClosing) {
            return false;
        }
        logInfo("process response");

        auto resp = std::get<1>(m_requestQueue.front());

        const bool result = resp(values...);
        logInfo(std::string("response validation result:") + (result ? "true" : "false"));

        if (result) {
            m_isWaitingForEvent = true;
        } else {
            m_isWaitingForEvent = false;

            logInfo("send failed responses");
            std::tuple<EvArgs...> defaultValues;
            VariadicCaller<EvArgs...>::invoke([this](EvArgs... args) { sendResults(args...); }, defaultValues);
        }

        return result;
    });
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>>::sendResults(EvArgs... args)
{
    while (!m_isClosing && !m_requestQueue.empty()) {
        auto ev = std::get<2>(m_requestQueue.front());
        m_requestQueue.pop_front();

        if (m_isPaused) {
            break;
        }

        ev(args...);
    }
}

} // namespace psi::comm