#pragma once

#include <functional>

#include "psi/comm/Subscription.h"

namespace psi::comm {

/**
 * @brief Attribute class is used for notification listeners on change its value
 * 
 * @tparam T 
 */
template <typename T>
class IAttribute
{
public:
    using EventFunc = std::function<void(T, T)>;

    /**
     * @brief Returns current value of attribute
     * 
     * @return const T& value
     */
    virtual const T &value() const = 0;

    /**
     * @brief Subscribes listener to attribute's change
     * 
     * @param func function to be called on attribute change
     * @return Subscription listener object
     */
    virtual Subscription subscribe(EventFunc &&func) = 0;

    /**
     * @brief Subscribes listener to attribute's change.
     * Callback will be sent immediately after subscription with current result.
     * 
     * @param func function to be called on attribute change
     * @return Subscription listener object
     */
    virtual Subscription subscribeAndGet(EventFunc &&func) = 0;
};

} // namespace psi::comm
