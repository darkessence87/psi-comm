
#pragma once

#include <atomic>
#include <deque>
#include <functional>

#include "psi/comm/call_strategy/BasicStrategy.h"

namespace psi::comm {

/**
 * @brief Fully-synchronous event-driven strategy.
 *
 * Two-phase serialised handshake:
 * 1. processRequest() dispatches one request at a time.  The request calls
 *    its @c ResponseFunc validation callback which returns @c true to signal
 *    acceptance and @c false to fail immediately.
 * 2. After a successful validation the strategy waits until processEvent() is
 *    called, then invokes the @c EvFunc (onEvent) callback and advances to the
 *    next queued request.
 *
 * Processing can be suspended with pauseRequest() and resumed with
 * continueRequest().
 */
template <typename... EvArgs, typename... CbArgs>
class EvStrategy<EvStrategyType::FullySync, TypeList<EvArgs...>, TypeList<CbArgs...>> : public BasicStrategy
{
public:
    using EvFunc = std::function<void(EvArgs...)>;
    using ResponseFunc = std::function<bool(CbArgs...)>;  ///< Validation callback; return true to accept.
    using RequestFunc = std::function<void(ResponseFunc)>;
    using QueuedRequest = std::tuple<RequestFunc, ResponseFunc, EvFunc>;

    EvStrategy(const std::string &logPrefix = "");
    virtual ~EvStrategy();

    /**
     * @brief Enqueue a two-phase request.
     *
     * If no request is active, @p request is dispatched immediately.
     * Otherwise it is appended to the internal queue.
     *
     * @param request   Callable that performs the operation and invokes
     *                  @p response as a validation step.
     * @param response  Validation callback; must return @c true to proceed to
     *                  the event-wait phase, or @c false to fail immediately.
     * @param onEvent   Callback invoked when processEvent() is called after a
     *                  successful validation.
     */
    void processRequest(RequestFunc &&request, ResponseFunc &&response, EvFunc &&onEvent);

    /**
     * @brief Signal that the external event has occurred.
     *
     * Invokes the @c onEvent callback of the currently-waiting request and
     * dispatches the next queued request if one exists.
     * Ignored if the strategy is not in the event-wait state.
     *
     * @param args  Arguments forwarded to the @c onEvent callback.
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

private:
    std::deque<QueuedRequest> m_requestQueue;
    std::atomic<bool> m_isWaitingForEvent = false;
    std::atomic<bool> m_isPaused = false;
    std::atomic<bool> m_isClosing = false;
};

template <typename EvArgsList, typename CbArgsList>
using FullySyncEvStrategy = EvStrategy<EvStrategyType::FullySync, EvArgsList, CbArgsList>;

} // namespace psi::comm