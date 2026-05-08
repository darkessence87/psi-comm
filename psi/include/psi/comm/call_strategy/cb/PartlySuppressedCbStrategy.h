
#pragma once

#include <functional>
#include <queue>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

/**
 * @brief Partly-suppressed-sync callback strategy.
 *
 * The first request is dispatched; subsequent in-flight requests are queued.
 * When the response arrives, the first callback and all **intermediate**
 * (suppressed) callbacks receive the result immediately, and then the **last**
 * queued request is dispatched as a brand-new request.  This ensures at most
 * two actual round-trips regardless of how many requests were accumulated.
 */
template <typename... CbArgs>
class CbStrategy<CbStrategyType::PartlySuppressedSync, TypeList<CbArgs...>> : public BasicStrategy
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
     * @brief Enqueue or dispatch a request with partly-suppressed semantics.
     *
     * If the queue is empty the request is dispatched immediately.  Otherwise
     * it is queued.  On response: the first and all intermediate (suppressed)
     * callbacks receive the result, then the last queued request is dispatched
     * as a new independent request.
     *
     * @param request  Callable that performs the operation and invokes its
     *                 first argument (the response callback) when done.
     * @param response Callback to invoke with the result.
     */
    void processRequest(RequestFunc &&request, ResponseFunc &&response);

private:
    void processNext();

private:
    std::queue<QueuedRequest> m_queue;
    std::atomic<bool> m_isClosing = false;
    std::atomic<bool> m_interruptImmediately = false;
};

template <typename... CbArgs>
using PartlySuppressedCbStrategy = CbStrategy<CbStrategyType::PartlySuppressedSync, TypeList<CbArgs...>>;

} // namespace psi::comm