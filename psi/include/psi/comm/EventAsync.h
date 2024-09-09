#pragma once

#include "psi/comm/Event.h"
#include "psi/comm/SafeCaller.h"

namespace psi::comm {

/**
 * @brief EventAsync class is used for asynchronous notification listeners
 * 
 * @tparam Args 
 */
template <typename Strategy, typename Event>
class EventAsync : public Event
{
public:
    /**
     * @brief Construct a new EventAsync object
     * 
     * @param strategy reference to strategy object
     */
    EventAsync(Strategy &strategy)
        : m_guard(this)
        , m_strategy(strategy)
    {
    }

    /**
     * @brief Notifies all listeners asynchronously.
     * It is safe to remove listener in a reaction.
     * 
     * @param args 
     */
    template <typename... Args>
    void notify(Args... args) const
    {
        m_strategy.asyncCall(m_guard.invoke([=]() { Event::notify(args...); }));
    }

protected:
    SafeCaller m_guard;
    Strategy &m_strategy;
};

} // namespace psi::comm
