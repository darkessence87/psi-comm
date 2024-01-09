
#pragma once

#include <sstream>

namespace psi::comm {

enum class CbStrategyType
{
    /// Async strategy
    ///     calls requests immediately
    ///     sends callbacks immediately on responses
    Async,

    /// FullySync strategy
    ///     calls requests only on response for previous request
    ///     sends callback immediately on response
    FullySync,

    /// PartlySuppressedSync strategy
    ///     calls first [1] request in queue
    ///     sends callbacks for [1,N-1] requests in queue immediately on response [1]
    ///     calls last queued [N] request
    ///     sends last callback [N]
    PartlySuppressedSync,

    /// SuppressedSync strategy
    ///     calls first [1] request in queue
    ///     sends callbacks for [1,N-1] requests in queue immediately on response [1]
    SuppressedSync,

    /// CachedAsync
    ///     requests, equal by input params, are called via SuppressedSync strategy
    ///     requests, not equal by input params, are called via Async strategy
    CachedAsync
};

inline std::ostream &operator<<(std::ostream &str, const CbStrategyType cs)
{
    switch (cs) {
    case CbStrategyType::Async:
        str << "Async";
        break;
    case CbStrategyType::FullySync:
        str << "FullySync";
        break;
    case CbStrategyType::PartlySuppressedSync:
        str << "PartlySuppressedSync";
        break;
    case CbStrategyType::SuppressedSync:
        str << "SuppressedSync";
        break;
    case CbStrategyType::CachedAsync:
        str << "CachedAsync";
        break;
    }
    return str;
}

inline std::string asString(CbStrategyType cs)
{
    std::ostringstream str;
    str << cs;
    return str.str();
}

} // namespace psi::comm