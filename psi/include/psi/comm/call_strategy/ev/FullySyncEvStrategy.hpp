
#pragma once

#include "FullySyncEvStrategy.h"

namespace psi::comm {

template <typename... EvArgs, typename... CbArgs>
EvStrategy<EvStrategyType::FullySync, TypeList<EvArgs...>, TypeList<CbArgs...>>::EvStrategy(const std::string &logPrefix)
    : BasicStrategy(asString(EvStrategyType::FullySync), logPrefix)
{
    logInfo("EvStrategy created");
}

template <typename... EvArgs, typename... CbArgs>
EvStrategy<EvStrategyType::FullySync, TypeList<EvArgs...>, TypeList<CbArgs...>>::~EvStrategy()
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
void EvStrategy<EvStrategyType::FullySync, TypeList<EvArgs...>, TypeList<CbArgs...>>::processRequest(RequestFunc &&request,
                                                                                                     ResponseFunc &&response,
                                                                                                     EvFunc &&onEvent)
{
    if (m_isClosing) {
        return;
    }

    if (!m_requestQueue.empty() || m_isPaused) {
        m_mutex.lock();
        m_requestQueue.emplace_back(QueuedRequest {request, response, onEvent});
        logInfo("queued request");
        m_mutex.unlock();
        return;
    }

    m_mutex.lock();
    m_requestQueue.emplace_back(QueuedRequest {request, response, onEvent});
    m_mutex.unlock();
    processNext();
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::FullySync, TypeList<EvArgs...>, TypeList<CbArgs...>>::processEvent(EvArgs... args)
{
    if (m_isClosing) {
        return;
    }

    if (!m_isWaitingForEvent) {
        logInfo("Not waiting for event. Ignore.");
        return;
    }

    m_isWaitingForEvent = false;

    if (!m_requestQueue.empty()) {
        logInfo("process event");

        m_mutex.lock();
        auto ev = std::get<2>(m_requestQueue.front());
        m_requestQueue.pop_front();
        const bool needProcessNext = !m_requestQueue.empty();
        m_mutex.unlock();

        ev(args...);

        if (needProcessNext && !m_isPaused) {
            processNext();
        }
    }
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::FullySync, TypeList<EvArgs...>, TypeList<CbArgs...>>::pauseRequest()
{
    if (m_isClosing) {
        return;
    }

    m_isPaused = true;

    logInfo("request paused");

    if (!m_requestQueue.empty()) {
        m_mutex.lock();
        auto requestCopy = m_requestQueue.front();
        std::get<2>(requestCopy) = [this](EvArgs...) { logInfo("ignored invalidated response"); };
        m_requestQueue.emplace_front(requestCopy);
        m_mutex.unlock();
    }
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::FullySync, TypeList<EvArgs...>, TypeList<CbArgs...>>::continueRequest()
{
    if (m_isClosing) {
        return;
    }

    m_isPaused = false;

    logInfo("request resumed");

    if (!m_requestQueue.empty() && !m_isWaitingForEvent) {
        processNext();
    }
}

template <typename... EvArgs, typename... CbArgs>
void EvStrategy<EvStrategyType::FullySync, TypeList<EvArgs...>, TypeList<CbArgs...>>::processNext()
{
    if (m_requestQueue.empty()) {
        return;
    }

    logInfo("process request");

    auto request = std::get<0>(m_requestQueue.front());
    request([this](CbArgs... values) -> bool {
        if (m_isClosing || m_requestQueue.empty()) {
            return false;
        }

        logInfo("process response");

        //m_mutex.lock();
        auto response = std::get<1>(m_requestQueue.front());
        //m_mutex.unlock();

        const bool result = response(values...);
        logInfo(std::string("response validation result:") + (result ? "true" : "false"));

        if (result) {
            m_isWaitingForEvent = true;
        } else {
            m_isWaitingForEvent = false;

            if (!m_requestQueue.empty()) {
                logInfo("send failed response");

                m_mutex.lock();
                auto ev = std::get<2>(m_requestQueue.front());
                m_requestQueue.pop_front();
                const bool needProcessNext = !m_requestQueue.empty();
                m_mutex.unlock();

                std::tuple<EvArgs...> defaultValues;
                VariadicCaller<EvArgs...>::invoke(ev, defaultValues);

                if (needProcessNext && !m_isPaused) {
                    processNext();
                }
            }
        }

        return result;
    });
}

} // namespace psi::comm