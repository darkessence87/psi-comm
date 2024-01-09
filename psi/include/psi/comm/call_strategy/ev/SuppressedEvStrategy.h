
#pragma once

#include <deque>
#include <functional>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

template <typename... EvArgs, typename... CbArgs>
class EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>> : public BasicStrategy
{
public:
    using EventFunc = std::function<void(EvArgs...)>;
    using ResponseFunc = std::function<bool(CbArgs...)>;
    using RequestFunc = std::function<void(ResponseFunc)>;
    using QueuedRequest = std::tuple<RequestFunc, ResponseFunc, EventFunc>;

    EvStrategy(const std::string &logPrefix = "");
    virtual ~EvStrategy();

    void processRequest(RequestFunc request, ResponseFunc response, EventFunc onEvent);
    void processEvent(EvArgs... args);
    void pauseRequest();
    void continueRequest();

private:
    void processNext();
    void sendResults(EvArgs... args);

private:
    std::deque<QueuedRequest> m_requestQueue;
    bool m_isWaitingForEvent = false;
    bool m_isPaused = false;
    std::atomic<bool> m_isClosing = false;
};

template <typename EvArgsList, typename CbArgsList>
using SuppressedEvStrategy = EvStrategy<EvStrategyType::SuppressedSync, EvArgsList, CbArgsList>;

} // namespace psi::comm