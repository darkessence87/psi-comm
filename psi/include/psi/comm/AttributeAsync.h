#pragma once

#include "psi/comm/Attribute.h"
#include "psi/comm/EventAsync.h"

namespace psi::comm {

/**
 * @brief AttributeAsync class is similar to Attribute but notification will be processed by provided strategy
 * 
 * @tparam T type of value
 */
template <typename T>
class AttributeAsync : public Attribute<T>
{
public:
    /**
     * @brief Construct a new AttributeAsync object
     * 
     * @tparam Strategy type of strategy object
     * @param strategy reference to strategy object
     */
    template <typename Strategy>
    AttributeAsync(Strategy &strategy)
        : Attribute<T>(std::make_unique<EventAsync<Strategy, Event<T, T>>>(strategy))
    {
    }
};

} // namespace psi::comm
