
#pragma once

#include <functional>
#include <map>

#include "FullySyncEvStrategy.h"
#include "psi/comm/call_strategy/Comparable.h"

namespace psi::comm {

template <typename InputComparable, typename... EvArgs, typename... CbArgs>
class EvStrategy<EvStrategyType::CachedSync, TypeList<EvArgs...>, TypeList<CbArgs...>, InputComparable> : public BasicStrategy
{
    struct CacheKey;
    struct CacheValue;

    using CacheMap = typename std::map<CacheKey, CacheValue>;
    using EvFunc = std::function<void(EvArgs...)>;
    using ResponseFunc = std::function<bool(CbArgs...)>;
    using RequestFunc = std::function<void(ResponseFunc)>;
    using Processor = FullySyncEvStrategy<TypeList<EvArgs...>, TypeList<CbArgs...>>;

public:
    EvStrategy(const std::string &logPrefix = "");
    virtual ~EvStrategy();

    /**
     * @brief Dispatch or serve from cache.
     *
     * The first call for a given @p in key routes the request through the
     * internal FullySyncEvStrategy.  Subsequent calls with the same key while
     * the result is pending register @p onEvent as an additional subscriber —
     * the underlying request is **not** re-dispatched.  Once the event fires
     * all accumulated callbacks are invoked.  Further calls with the same key
     * after the result is cached invoke @p onEvent immediately.
     *
     * @param in        Key used to identify and deduplicate requests.
     * @param request   Callable dispatched at most once per cache key.
     * @param response  Validation callback forwarded to the underlying strategy.
     * @param onEvent   Callback invoked when the event fires (immediately if
     *                  the result is already cached).
     */
    void processRequest(const InputComparable &in, RequestFunc &&request, ResponseFunc &&response, EvFunc &&onEvent);

    /**
     * @brief Forward the external event to the underlying FullySyncEvStrategy.
     *
     * This advances the in-progress request to its completion phase, which
     * stores the result in the cache and invokes all registered callbacks.
     *
     * @param args  Arguments forwarded to every registered @c onEvent callback.
     */
    void processEvent(EvArgs... args);

    /**
     * @brief Clear the result cache and reset the internal sync processor.
     *
     * All cached results are discarded.  In-flight requests are abandoned;
     * their callbacks will not be invoked after reset.
     */
    void reset();

private:
    void onResponse(CacheKey key, EvArgs... result);

private:
    CacheMap m_cacheMap;
    std::unique_ptr<Processor> m_processor;
};

template <typename InputComparable, typename EvArgsList, typename CbArgsList>
using CachedEvStrategy = EvStrategy<EvStrategyType::CachedSync, EvArgsList, CbArgsList, InputComparable>;

} // namespace psi::comm