
#pragma once

#include "AsyncCbStrategy.h"

namespace psi::comm {

template <typename... CbArgs>
CbStrategy<CbStrategyType::Async, TypeList<CbArgs...>>::CbStrategy(const std::string &logPrefix)
    : BasicStrategy(asString(CbStrategyType::Async), logPrefix)
{
    logInfo("CbStrategy created");
}

template <typename... CbArgs>
CbStrategy<CbStrategyType::Async, TypeList<CbArgs...>>::~CbStrategy()
{
    logInfo("CbStrategy deleted");
}

template <typename... CbArgs>
void CbStrategy<CbStrategyType::Async, TypeList<CbArgs...>>::processRequest(RequestFunc request, ResponseFunc response)
{
    logInfo("process request");
    request(response);
}

} // namespace psi::comm