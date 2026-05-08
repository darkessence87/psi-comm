
#pragma once

#include <atomic>
#include <functional>
#include <queue>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

/**
 * @brief Fully-synchronous (serialised) callback strategy.
 *
 * Only one request is active at a time.  While a request is waiting for its
 * response callback, all subsequent requests are queued and dispatched one by
 * one as each response arrives.
 */
template <typename... CbArgs>
class CbStrategy<CbStrategyType::FullySync, TypeList<CbArgs...>> : public BasicStrategy
{
public:
    using ResponseFunc = std::function<void(CbArgs...)>;
    using RequestFunc = std::function<void(ResponseFunc)>;
    using QueuedRequest = std::pair<RequestFunc, ResponseFunc>;

    CbStrategy(const std::string &logPrefix = "");
    virtual ~CbStrategy();

    /**
     * @brief Flush all pending responses with default-constructed values.
     *
     * Marks the strategy as closing and drains the queue by calling every
     * queued response with a zero-initialised argument tuple.  No further
     * requests will be dispatched after this call.
     */
    void interrupt();

    /**
     * @brief Discard all pending requests and responses without calling them.
     *
     * Sets the internal interrupt-immediately flag before delegating to
     * interrupt(), which then skips the response callbacks entirely.
     */
    void interruptImmediately();

    /**
     * @brief Enqueue a request for serialised dispatch.
     *
     * If no request is currently active, @p request is dispatched immediately.
     * Otherwise it is appended to the internal queue and will be dispatched
     * once all preceding requests have received their responses.
     *
     * @param request  Callable that performs the operation and invokes its
     *                 first argument (the response callback) when done.
     * @param response Callback invoked with the result of this specific request.
     */
    void processRequest(RequestFunc request, ResponseFunc response);

private:
    void processNext();

private:
    std::queue<QueuedRequest> m_queue;
    std::atomic<bool> m_isClosing = false;
    std::atomic<bool> m_interruptImmediately = false;
};

template <typename... CbArgs>
using FullySyncCbStrategy = CbStrategy<CbStrategyType::FullySync, TypeList<CbArgs...>>;

} // namespace psi::comm
