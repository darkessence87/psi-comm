
#pragma once

#include <functional>
#include <queue>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

template <typename... CbArgs>
class CbStrategy<CbStrategyType::SuppressedSync, TypeList<CbArgs...>> : public BasicStrategy
{
public:
    using ResponseFunc = std::function<void(CbArgs...)>;
    using RequestFunc = std::function<void(ResponseFunc)>;
    using QueuedRequest = std::pair<RequestFunc, ResponseFunc>;

    CbStrategy(const std::string &logPrefix = "");
    virtual ~CbStrategy();

    void interrupt();
    void processRequest(RequestFunc request, ResponseFunc response);

private:
    std::queue<QueuedRequest> m_queue;
    std::atomic<bool> m_isClosing = false;
};

template <typename... CbArgs>
using SuppressedCbStrategy = CbStrategy<CbStrategyType::SuppressedSync, TypeList<CbArgs...>>;

} // namespace psi::comm
