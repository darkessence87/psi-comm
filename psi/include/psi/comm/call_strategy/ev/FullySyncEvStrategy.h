
#pragma once

#include <atomic>
#include <deque>
#include <functional>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

template <typename... EvArgs, typename... CbArgs>
class EvStrategy<EvStrategyType::FullySync, TypeList<EvArgs...>, TypeList<CbArgs...>> : public BasicStrategy
{
public:
    using EvFunc = std::function<void(EvArgs...)>;
    using ResponseFunc = std::function<bool(CbArgs...)>;
    using RequestFunc = std::function<void(ResponseFunc)>;
    using QueuedRequest = std::tuple<RequestFunc, ResponseFunc, EvFunc>;

    EvStrategy(const std::string &logPrefix = "");
    virtual ~EvStrategy();

    void processRequest(RequestFunc &&request, ResponseFunc &&response, EvFunc &&onEvent);
    void processEvent(EvArgs... args);
    void pauseRequest();
    void continueRequest();

private:
    void processNext();

private:
    std::deque<QueuedRequest> m_requestQueue;
    std::atomic<bool> m_isWaitingForEvent = false;
    std::atomic<bool> m_isPaused = false;
    std::atomic<bool> m_isClosing = false;
};

template <typename EvArgsList, typename CbArgsList>
using FullySyncEvStrategy = EvStrategy<EvStrategyType::FullySync, EvArgsList, CbArgsList>;

} // namespace psi::comm