#pragma once

#include <functional>

#include "Subscription.h"

namespace psi::comm {

/**
 * @brief IEvent provides interface for subscribe any number of listeners to notifications
 * 
 * @tparam Args 
 */
template <typename... Args>
class IEvent
{
public:
    /// @brief type of function is used for notifications
    using Func = std::function<void(Args...)>;

    /// @brief Subscribes a function to be called whenever event is sent
    /// @param fn function to be called whenever event is sent
    /// @return listener object, as long as listener object exists event will notify it
    virtual Subscription subscribe(Func &&fn) = 0;
};

} // namespace psi::comm
