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
    using ListenersList = std::list<WeakSubscription>;
    using Func = typename Interface::Func;

    /**
     * @brief Listener will automatically unsubscribe from event if it is destroyed
     * 
     */
    struct Listener final : std::enable_shared_from_this<Listener>, Subscribable {
        /// @brief Unique id of listener, in fact it is iterator of holder's list
        using Identifier = typename std::list<WeakSubscription>::reverse_iterator;

        /// @brief Constructs listener object
        /// @param holder reference to holder of all listeners
        /// @param fn function to be called whenever event is sent
        Listener(std::weak_ptr<ListenersList> holder, Func &&fn)
            : m_holder(holder)
            , m_function(std::forward<Func>(fn))
        {
        }

        /// @brief Destroys the listener and removes it from holder's list
        ~Listener()
        {
            if (auto holder = m_holder.lock()) {
                holder->erase(std::next(m_identifier).base());
            }
        }

    private:
        std::weak_ptr<ListenersList> m_holder;
        Identifier m_identifier;
        Func m_function;

        friend class Event<Args...>;
    };

public:
    Event() : m_listeners(std::make_shared<ListenersList>())
    {
    }

    ~Event()
    {
        m_listeners->clear();
    }

    /**
     * @brief Notifies all listeners.
     * It is safe to remove listener in a reaction.
     * 
     * @param args 
     */
    virtual void notify(Args... args) const
    {
        auto copy = *m_listeners.get();
        for (auto itr = copy.begin(); itr != copy.end(); ++itr) {
            if (auto ptr = itr->lock()) {
                ptr->m_function(args...);
            }
        }
    }

    /**
     * @brief Creates listener to event's notifications.
     * 
     * @param fn function to be called on event sent
     * @return Subscription listener object, as long as listener object exists event will notify it
     */
    Subscription subscribe(const Func &fn) override
    {
        auto listener = createListener();
        listener->m_function = fn;
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
        m_listeners->emplace_back(listener);
        listener->m_identifier = m_listeners->rbegin();
        return listener;
    }

protected:
    std::shared_ptr<ListenersList> m_listeners;
};

} // namespace psi::comm
