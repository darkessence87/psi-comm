
#pragma once

#include "AsyncCbStrategy.h"

namespace psi::comm {

template <typename... CbArgs>
AsyncCbStrategy<CbArgs...>::CbStrategy(const std::string &logPrefix)
    : BasicStrategy(asString(CbStrategyType::Async), logPrefix)
{
    logInfo("CbStrategy created");
}

template <typename... CbArgs>
AsyncCbStrategy<CbArgs...>::~CbStrategy()
{
    logInfo("CbStrategy deleted");
}

template <typename... CbArgs>
void AsyncCbStrategy<CbArgs...>::processRequest(RequestFunc request, ResponseFunc response)
{
    logInfo("process request");
    request(response);
}

} // namespace psi::comm