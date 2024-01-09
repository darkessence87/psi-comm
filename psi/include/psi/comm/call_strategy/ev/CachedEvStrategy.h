
#pragma once

#include <functional>
#include <map>

#include "FullySyncEvStrategy.h"
#include "psi/comm/call_strategy/Comparable.h"

namespace psi::comm {

template <typename... EvArgs, typename... CbArgs, typename InputComparable>
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

    void processRequest(const InputComparable &in, RequestFunc &&request, ResponseFunc &&response, EvFunc &&onEvent);
    void processEvent(EvArgs... args);
    void reset();

private:
    void onResponse(CacheKey key, EvArgs... result);

private:
    CacheMap m_cacheMap;
    std::unique_ptr<Processor> m_processor;
};

template <typename InputParam, typename EvArgsList, typename CbArgsList>
using CachedEvStrategy = EvStrategy<EvStrategyType::CachedSync, EvArgsList, CbArgsList, InputParam>;

} // namespace psi::comm