#pragma once

#include <iostream>
#include <list>

#include "psi/comm/IEvent.h"

namespace psi::comm {

/**
 * @brief Event class is used for notification listeners
 * 
 * @tparam Args 
 */
template <typename... Args>
class Event : public IEvent<Args...>
{
public:
    /// @brief Short alias to interface. Interface has to be provided to other clients who should only listen to event
    using Interface = IEvent<Args...>;

    class Listener;
    using WeakSubscription = std::weak_ptr<Listener>;
    using Func = typename Interface::Func;

    /**
     * @brief Listener will automatically unsubscribe from event if it is destroyed
     * 
     */
    struct Listener final : std::enable_shared_from_this<Listener>, Subscribable {
        /// @brief Unique id of listener, in fact it is iterator of holder's list
        using Identifier = typename std::list<WeakSubscription>::iterator;

        /// @brief Constructs listener object
        /// @param holder reference to holder of all listeners
        /// @param fn function to be called whenever event is sent
        Listener(std::list<WeakSubscription> &holder, Func &&fn)
            : m_holder(holder)
            , m_function(std::forward<Func>(fn))
        {
        }

        /// @brief Destroys the listener and removes it from holder's list
        ~Listener()
        {
            m_holder.erase(m_identifier);
        }

        /// @brief Reaction may be changed any time between event's notifications
        /// @param fn new function to be called on event sent
        void setFunction(Func &&fn)
        {
            m_function = std::forward<Func>(fn);
        }

    private:
        std::list<WeakSubscription> &m_holder;
        Identifier m_identifier;
        Func m_function;

        friend class Event<Args...>;
    };

public:
    /**
     * @brief Notifies all listeners.
     * It is safe to remove listener in a reaction.
     * 
     * @param args 
     */
    virtual void notify(Args... args)
    {
        auto copy = m_listeners;
        for (auto itr = copy.begin(); itr != copy.end(); ++itr) {
            if (auto ptr = itr->lock()) {
                ptr->m_function(std::forward<Args>(args)...);
            } else {
                throw std::domain_error("Listener has no ptr!");
            }
        }
    }

    /**
     * @brief Creates listener to event's notifications.
     * 
     * @param fn function to be called on event sent
     * @return Subscription listener object, as long as listener object exists event will notify it
     */
    Subscription subscribe(Func &&fn) override
    {
        auto listener = createListener();
        listener->setFunction(std::forward<Func>(fn));
        return std::dynamic_pointer_cast<Subscribable>(listener);
    }

    /**
     * @brief Creates listener with default reaction on event's notification.
     * Listener's reaction must be replaced later by client.
     * 
     * @return std::shared_ptr<Listener> pointer to listener object
     */
    std::shared_ptr<Listener> createListener()
    {
        auto listener =
            std::make_shared<Listener>(m_listeners, [this](Args &&...) { std::cerr << "Not implemented!" << std::endl; });
        m_listeners.emplace_front(listener);
        listener->m_identifier = m_listeners.begin();
        return listener;
    }

protected:
    std::list<WeakSubscription> m_listeners;
};

} // namespace psi::comm
