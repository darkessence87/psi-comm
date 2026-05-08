
#pragma once

#include <functional>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

/**
 * @brief Asynchronous callback strategy.
 *
 * Every call to processRequest() dispatches the request immediately without
 * any serialisation.  Multiple requests may be in-flight simultaneously and
 * their responses may arrive in any order.
 */
template <typename... CbArgs>
class CbStrategy<CbStrategyType::Async, TypeList<CbArgs...>> : public BasicStrategy
{
public:
    using ResponseFunc = std::function<void(CbArgs...)>;
    using RequestFunc = std::function<void(ResponseFunc)>;

    CbStrategy(const std::string &logPrefix = "");
    virtual ~CbStrategy();

    /**
     * @brief Dispatch a request immediately.
     *
     * @p request is called at once with @p response wrapped as the callback.
     * There is no queuing; concurrent calls proceed in parallel.
     *
     * @param request  Callable that performs the operation and invokes its
     *                 first argument (the response callback) when done.
     * @param response Callback invoked by the request with the result.
     */
    void processRequest(RequestFunc request, ResponseFunc response);
};

template <typename... CbArgs>
using AsyncCbStrategy = CbStrategy<CbStrategyType::Async, TypeList<CbArgs...>>;

} // namespace psi::comm
