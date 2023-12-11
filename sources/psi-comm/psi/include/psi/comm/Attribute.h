#pragma once

#include "psi/comm/Event.h"
#include "psi/comm/IAttribute.h"

namespace psi::comm {

/**
 * @brief Attribute class is used for notification listeners on change its value
 * 
 * @tparam T type of value
 */
template <typename T>
class Attribute : public IAttribute<T>
{
public:
    /// @brief Short alias to interface. Interface has to be provided to other clients who should have read-only access
    using Interface = IAttribute<T>;

    using Listener = typename Event<T, T>::Listener;
    using EventFunc = typename Interface::EventFunc;

    /**
     * @brief Construct a new Attribute object
     * 
     * @param defaultValue initial value of attribute
     */
    Attribute(T &&defaultValue = T())
        : m_value(std::forward<T>(defaultValue))
    {
    }

    /**
     * @brief Returns current value of attribute
     * 
     * @return const T& value
     */
    const T &value() const override
    {
        return m_value;
    }

    /**
     * @brief Sets new value of Attribute<T>. If new value is not equal to old value notification will be sent to listeners.
     * Notification contains old and new values.
     * Notification will be sent after new value is saved in Attribute<T>.
     * 
     * @param value 
     */
    void setValue(T &&value)
    {
        const bool needNotification = m_value != value;
        const auto oldValue = m_value;

        m_value = std::forward<T>(value);

        if (needNotification) {
            m_event.notify(oldValue, m_value);
        }
    }

    /**
     * @brief Subscribes listener to attribute's change
     * 
     * @param func function to be called on attribute change
     * @return Subscription listener object
     */
    Subscription subscribe(EventFunc &&func) override
    {
        return m_event.subscribe(std::forward<EventFunc>(func));
    }

    /**
     * @brief Subscribes listener to attribute's change.
     * Callback will be sent immediately after subscription with current result.
     * 
     * @param func function to be called on attribute change
     * @return Subscription listener object
     */
    Subscription subscribeAndGet(EventFunc &&func) override
    {
        func(m_value, m_value);
        return subscribe(std::forward<EventFunc>(func));
    }

    ///
    /// Creates listener with default reaction on attribute's change.
    /// Listener's reaction must be replaced later by client.
    ///

    /**
     * @brief Creates listener with default reaction on attribute's change.
     * Listener's reaction must be replaced later by client.
     * 
     * @return std::shared_ptr<Listener> pointer to listener object
     */
    std::shared_ptr<Listener> createListener()
    {
        return m_event.createListener();
    }

private:
    T m_value;
    Event<T /*old value*/, T /*new value*/> m_event;
};

} // namespace psi::comm
