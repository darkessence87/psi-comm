
#pragma once

#include <functional>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

template <typename... CbArgs>
class CbStrategy<CbStrategyType::Async, TypeList<CbArgs...>> : public BasicStrategy
{
public:
    using ResponseFunc = std::function<void(CbArgs...)>;
    using RequestFunc = std::function<void(ResponseFunc)>;

    CbStrategy(const std::string &logPrefix = "");
    virtual ~CbStrategy();

    void processRequest(RequestFunc request, ResponseFunc response);
};

template <typename... CbArgs>
using AsyncCbStrategy = CbStrategy<CbStrategyType::Async, TypeList<CbArgs...>>;

} // namespace psi::comm