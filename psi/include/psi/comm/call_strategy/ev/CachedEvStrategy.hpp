
#pragma once

#include "CachedEvStrategy.h"
#include "FullySyncEvStrategy.hpp"

#include <optional>

namespace psi::comm {

template <typename... EvArgs, typename... CbArgs, typename InputComparable>
struct CachedEvStrategy<InputComparable, TypeList<EvArgs...>, TypeList<CbArgs...>>::CacheKey {
    static_assert(std::is_base_of<Comparable, InputComparable>() || std::is_standard_layout<InputComparable>(),
                  "In CacheKey type <InputComparable> must inherite 'struct Comparable'");

    InputComparable key;

    friend bool operator<(const CacheKey &k1, const CacheKey &k2)
    {
        return k1.key < k2.key;
    }
};

template <typename... EvArgs, typename... CbArgs, typename InputComparable>
struct CachedEvStrategy<InputComparable, TypeList<EvArgs...>, TypeList<CbArgs...>>::CacheValue {
    using Func = std::function<void(EvArgs...)>;
    std::optional<std::tuple<EvArgs...>> result;
    std::list<Func> callbacks;

    template <typename Func>
    void invoke(Func fn)
    {
        VariadicCaller<EvArgs...>::invoke(fn, result.value());
    }
};

template <typename... EvArgs, typename... CbArgs, typename InputComparable>
CachedEvStrategy<InputComparable, TypeList<EvArgs...>, TypeList<CbArgs...>>::EvStrategy(const std::string &logPrefix)
    : BasicStrategy(asString(EvStrategyType::CachedSync), logPrefix)
    , m_processor(std::make_unique<Processor>(logPrefix.empty() ? "" : logPrefix + "Cached"))
{
    logInfo("EvStrategy created");
}

template <typename... EvArgs, typename... CbArgs, typename InputComparable>
CachedEvStrategy<InputComparable, TypeList<EvArgs...>, TypeList<CbArgs...>>::~EvStrategy()
{
    logInfo("EvStrategy deleted");
}

template <typename... EvArgs, typename... CbArgs, typename InputComparable>
void CachedEvStrategy<InputComparable, TypeList<EvArgs...>, TypeList<CbArgs...>>::processRequest(const InputComparable &in,
                                                                                                 RequestFunc &&request,
                                                                                                 ResponseFunc &&response,
                                                                                                 EvFunc &&onEvent)
{
    CacheKey key {in};
    auto itr = m_cacheMap.find(key);
    if (itr == m_cacheMap.end()) {
        // not found cache -> create new one with captured key for sync
        logInfo("Not found cache -> create new one with captured key for sync");

        CacheValue value;
        value.result = std::nullopt;
        value.callbacks.emplace_back(onEvent);
        m_cacheMap[key] = value;

        m_processor->processRequest(std::move(request), std::move(response), [this, key](EvArgs... result) {
            onResponse(key, result...);
        });

        return;
    }

    // check if cached value exists
    auto &result = itr->second.result;
    if (!result.has_value()) {
        // add callback
        logInfo("add callback");
        itr->second.callbacks.emplace_back(onEvent);
        return;
    }

    // return callback immediately
    logInfo("return callback immediately");
    itr->second.invoke(onEvent);
}

template <typename... EvArgs, typename... CbArgs, typename InputComparable>
void CachedEvStrategy<InputComparable, TypeList<EvArgs...>, TypeList<CbArgs...>>::processEvent(EvArgs... args)
{
    m_processor->processEvent(args...);
}

template <typename... EvArgs, typename... CbArgs, typename InputComparable>
void CachedEvStrategy<InputComparable, TypeList<EvArgs...>, TypeList<CbArgs...>>::reset()
{
    m_processor.reset(new Processor());
    m_cacheMap.clear();
}

template <typename... EvArgs, typename... CbArgs, typename InputComparable>
void CachedEvStrategy<InputComparable, TypeList<EvArgs...>, TypeList<CbArgs...>>::onResponse(CacheKey key, EvArgs... result)
{
    auto itr = m_cacheMap.find(key);
    if (itr == m_cacheMap.end()) {
        logInfo("unsync callback with key or expired fallback");
        return;
    }

    auto &res = itr->second.result;
    res = std::tuple<EvArgs...>(result...);

    auto &callbacks = itr->second.callbacks;
    for (auto &cb : callbacks) {
        logInfo("call saved callback");
        cb(result...);
    }

    callbacks.clear();
}

} // namespace psi::comm