
#pragma once

#include <sstream>

namespace psi::comm {

enum class EvStrategyType
{
    /// FullySync strategy
    ///     calls requests only after event for previous request
    ///     calls requests only after failed response
    ///     sends callback immediately on failed response
    ///     sends callback immediately on event for request
    FullySync,

    /// SuppressedSync strategy
    ///     calls first [1] request in queue
    ///     sends callbacks for [1,N] requests in queue immediately on event for request
    SuppressedSync,

    /// CachedSync
    ///     requests, equal by input params, are called via SuppressedSync strategy
    ///     requests, not equal by input params, are called via FullySync strategy
    CachedSync
};

inline std::ostream &operator<<(std::ostream &str, const EvStrategyType es)
{
    switch (es) {
    case EvStrategyType::FullySync:
        str << "FullySync";
        break;
    case EvStrategyType::SuppressedSync:
        str << "SuppressedSync";
        break;
    case EvStrategyType::CachedSync:
        str << "CachedSync";
        break;
    }
    return str;
}

inline std::string asString(EvStrategyType cs)
{
    std::ostringstream str;
    str << cs;
    return str.str();
}

} // namespace psi::comm