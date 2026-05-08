
#pragma once

#include <functional>
#include <map>

#include "AsyncCbStrategy.h"
#include "psi/comm/call_strategy/Comparable.h"

namespace psi::comm {

template <typename InputComparable, typename... CbArgs>
class CbStrategy<CbStrategyType::CachedAsync, TypeList<CbArgs...>, InputComparable> : public BasicStrategy
{
    struct CacheKey;
    struct CacheValue;

    using CacheMap = typename std::map<CacheKey, CacheValue>;
    using ResponseFunc = std::function<void(CbArgs...)>;
    using RequestFunc = std::function<void(ResponseFunc)>;
    using Processor = AsyncCbStrategy<CbArgs...>;

public:
    CbStrategy(const std::string &logPrefix = "");
    virtual ~CbStrategy();

    /**
     * @brief Dispatch or serve from cache.
     *
     * The first call for a given @p in key dispatches @p request
     * asynchronously.  Subsequent calls with the same key while the request is
     * still in-flight register @p response as an additional callback — the
     * underlying request is **not** re-dispatched.  Once the result is
     * available, all accumulated callbacks are invoked.  Further calls with
     * the same key after the result is cached invoke @p response immediately.
     *
     * @param in       Key used to identify and deduplicate requests.
     * @param request  Callable dispatched at most once per cache key.
     * @param response Callback invoked with the result (immediately if cached).
     */
    void processRequest(const InputComparable &in, RequestFunc &&request, ResponseFunc &&response);

    /**
     * @brief Clear the result cache and reset the internal async processor.
     *
     * All cached results are discarded.  In-flight requests are abandoned;
     * their callbacks will not be invoked after reset.
     */
    void reset();

private:
    void onResponse(CacheKey key, CbArgs... result);

private:
    CacheMap m_cacheMap;
    std::unique_ptr<Processor> m_processor;
};

template <typename InputComparable, typename... CbArgs>
using CachedCbStrategy = CbStrategy<CbStrategyType::CachedAsync, TypeList<CbArgs...>, InputComparable>;

} // namespace psi::comm