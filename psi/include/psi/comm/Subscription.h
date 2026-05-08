#pragma once

#include <memory>

namespace psi::comm {

/**
 * @brief Subscribable class is used for generalization all types of subscriptions
 * 
 */
class Subscribable
{
public:
    virtual ~Subscribable() = default;
};

/**
 * @brief RAII subscription handle.
 *
 * Returned by subscribe() methods.  The subscription remains active as long
 * as this shared_ptr is alive; destroying or resetting it automatically
 * unsubscribes the associated listener.
 */
using Subscription = std::shared_ptr<Subscribable>;

} // namespace psi::comm
