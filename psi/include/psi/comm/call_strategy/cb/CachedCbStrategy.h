
#pragma once

#include <functional>
#include <map>

#include "AsyncCbStrategy.h"
#include "psi/comm/call_strategy/Comparable.h"

namespace psi::comm {

template <typename... CbArgs, typename InputComparable>
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

    void processRequest(const InputComparable &in, RequestFunc &&request, ResponseFunc &&response);
    void reset();

private:
    void onResponse(CacheKey key, CbArgs... result);

private:
    CacheMap m_cacheMap;
    std::unique_ptr<Processor> m_processor;
};

template <typename InputParam, typename... CbArgs>
using CachedCbStrategy = CbStrategy<CbStrategyType::CachedAsync, TypeList<CbArgs...>, InputParam>;

} // namespace psi::comm