
#pragma once

#include "AsyncCbStrategy.hpp"
#include "CachedCbStrategy.h"

#include <optional>

namespace psi::comm {

template <typename... CbArgs, typename InputComparable>
struct CachedCbStrategy<InputComparable, CbArgs...>::CacheKey {
    static_assert(std::is_base_of<Comparable, InputComparable>() || std::is_standard_layout<InputComparable>(),
                  "In CacheKey type <InputComparable> must inherite 'struct Comparable'");

    InputComparable key;

    friend bool operator<(const CacheKey &k1, const CacheKey &k2)
    {
        return k1.key < k2.key;
    }
};

template <typename... CbArgs, typename InputComparable>
struct CachedCbStrategy<InputComparable, CbArgs...>::CacheValue {
    using Func = std::function<void(CbArgs...)>;
    std::optional<std::tuple<CbArgs...>> result;
    std::list<Func> callbacks;

    template <typename Func>
    void invoke(Func fn)
    {
        VariadicCaller<CbArgs...>::invoke(fn, result.value());
    }
};

template <typename... CbArgs, typename InputComparable>
CachedCbStrategy<InputComparable, CbArgs...>::CbStrategy(const std::string &logPrefix)
    : BasicStrategy(asString(CbStrategyType::CachedAsync), logPrefix)
    , m_processor(std::make_unique<Processor>(logPrefix.empty() ? "" : logPrefix + "Cached"))
{
    logInfo("CbStrategy created");
}

template <typename... CbArgs, typename InputComparable>
CachedCbStrategy<InputComparable, CbArgs...>::~CbStrategy()
{
    logInfo("CbStrategy deleted");
}

template <typename... CbArgs, typename InputComparable>
void CachedCbStrategy<InputComparable, CbArgs...>::processRequest(const InputComparable &in,
                                                                  RequestFunc &&request,
                                                                  ResponseFunc &&response)
{
    CacheKey key {in};
    auto itr = m_cacheMap.find(key);
    if (itr == m_cacheMap.end()) {
        // not found cache -> create new one with captured key for sync
        logInfo("Not found cache -> create new one with captured key for sync");

        CacheValue value;
        value.result = std::nullopt;
        value.callbacks.emplace_back(response);
        m_cacheMap[key] = value;

        m_processor->processRequest(std::move(request), [this, key](CbArgs... result) { onResponse(key, result...); });

        return;
    }

    // check if cached value exists
    auto &result = itr->second.result;
    if (!result.has_value()) {
        // add callback
        logInfo("add callback");
        itr->second.callbacks.emplace_back(response);
        return;
    }

    // return callback immediately
    logInfo("return callback immediately");
    itr->second.invoke(response);
}

template <typename... CbArgs, typename InputComparable>
void CachedCbStrategy<InputComparable, CbArgs...>::reset()
{
    m_processor.reset(new Processor());
    m_cacheMap.clear();
}

template <typename... CbArgs, typename InputComparable>
void CachedCbStrategy<InputComparable, CbArgs...>::onResponse(CacheKey key, CbArgs... result)
{
    auto itr = m_cacheMap.find(key);
    if (itr == m_cacheMap.end()) {
        logInfo("unsync callback with key or expired fallback");
        return;
    }

    auto &res = itr->second.result;
    res = std::tuple<CbArgs...>(result...);

    auto &callbacks = itr->second.callbacks;
    for (auto &cb : callbacks) {
        logInfo("call saved callback");
        cb(result...);
    }

    callbacks.clear();
}

} // namespace psi::comm
