
#pragma once

#include <functional>
#include <queue>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

/**
 * @brief Suppressed-sync callback strategy.
 *
 * The first request is dispatched immediately; all subsequent requests that
 * arrive while the first is still in-flight are queued but never individually
 * dispatched.  When the single response arrives it is forwarded to **all**
 * queued callbacks (including the suppressed ones) with the same result values.
 */
template <typename... CbArgs>
class CbStrategy<CbStrategyType::SuppressedSync, TypeList<CbArgs...>> : public BasicStrategy
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
     * @brief Enqueue or immediately dispatch a request.
     *
     * If the queue is empty, @p request is dispatched straight away and its
     * response callback, when called, will fan out to all pending @p response
     * callbacks accumulated while the request was in-flight.
     * If the queue is non-empty, only @p response is stored — @p request is
     * never invoked.
     *
     * @param request  Callable that performs the operation and invokes its
     *                 first argument (the shared response callback) when done.
     * @param response Callback to invoke when the single in-flight response
     *                 is received.
     */
    void processRequest(RequestFunc request, ResponseFunc response);

private:
    std::queue<QueuedRequest> m_queue;
    std::atomic<bool> m_isClosing = false;
    std::atomic<bool> m_interruptImmediately = false;
};

template <typename... CbArgs>
using SuppressedCbStrategy = CbStrategy<CbStrategyType::SuppressedSync, TypeList<CbArgs...>>;

} // namespace psi::comm
