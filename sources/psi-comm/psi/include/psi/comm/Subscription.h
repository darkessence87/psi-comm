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

using Subscription = std::shared_ptr<Subscribable>;

} // namespace psi::comm
