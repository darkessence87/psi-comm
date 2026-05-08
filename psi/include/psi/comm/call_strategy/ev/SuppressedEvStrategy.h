
#pragma once

#include <deque>
#include <functional>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

/**
 * @brief Suppressed-sync event-driven strategy.
 *
 * Only the first request is dispatched; all subsequent requests that arrive
 * while it is in-flight are queued but never individually dispatched.
 * A single call to processEvent() fans out the event arguments to **all**
 * accumulated @c onEvent callbacks at once, then clears the queue.
 *
 * Processing can be suspended with pauseRequest() and resumed with
 * continueRequest().
 */
template <typename... EvArgs, typename... CbArgs>
class EvStrategy<EvStrategyType::SuppressedSync, TypeList<EvArgs...>, TypeList<CbArgs...>> : public BasicStrategy
{
public:
    using EventFunc = std::function<void(EvArgs...)>;
    using ResponseFunc = std::function<bool(CbArgs...)>;  ///< Validation callback; return true to accept.
    using RequestFunc = std::function<void(ResponseFunc)>;
    using QueuedRequest = std::tuple<RequestFunc, ResponseFunc, EventFunc>;

    EvStrategy(const std::string &logPrefix = "");
    virtual ~EvStrategy();

    /**
     * @brief Enqueue a request with suppressed-sync semantics.
     *
     * If the queue is empty, @p request is dispatched immediately.
     * Otherwise only @p onEvent is stored — @p request is never invoked.
     *
     * @param request   Callable that performs the operation and invokes
     *                  @p response as a validation step.
     * @param response  Validation callback; must return @c true to enter the
     *                  event-wait phase, or @c false to fail immediately.
     * @param onEvent   Callback invoked (along with all other suppressed
     *                  callbacks) when processEvent() is called.
     */
    void processRequest(RequestFunc request, ResponseFunc response, EventFunc onEvent);

    /**
     * @brief Signal the external event and flush all pending callbacks.
     *
     * Forwards @p args to every queued @c onEvent callback in FIFO order.
     * Ignored if the strategy is not in the event-wait state.
     *
     * @param args  Arguments forwarded to every @c onEvent callback.
     */
    void processEvent(EvArgs... args);

    /**
     * @brief Suspend dispatching of new requests.
     *
     * The currently active request is not interrupted; subsequent requests
     * will not be started until continueRequest() is called.
     */
    void pauseRequest();

    /**
     * @brief Resume dispatching after a previous pauseRequest().
     *
     * If requests are queued and the strategy is not waiting for an event,
     * the next request is dispatched immediately.
     */
    void continueRequest();

private:
    void processNext();
    void sendResults(EvArgs... args);

private:
    std::deque<QueuedRequest> m_requestQueue;
    bool m_isWaitingForEvent = false;
    bool m_isPaused = false;
    std::atomic<bool> m_isClosing = false;
};

template <typename EvArgsList, typename CbArgsList>
using SuppressedEvStrategy = EvStrategy<EvStrategyType::SuppressedSync, EvArgsList, CbArgsList>;

} // namespace psi::comm