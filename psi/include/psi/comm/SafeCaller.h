#pragma once

#include <functional>
#include <memory>

#include "psi/tools/Tools.h"

#ifdef PSI_LOGGER
#include "psi/logger/Logger.h"
#else
#include <iostream>
#include <sstream>
#define LOG_ERROR_STATIC(x)                                                                                            \
    do {                                                                                                               \
        std::ostringstream os;                                                                                         \
        os << x;                                                                                                       \
        std::cout << os.str() << std::endl;                                                                            \
    } while (0)
#endif

namespace psi::comm {

/**
 * @brief SafeCaller class is used for prevent crashes on calling member functions after the member have been destroyed.
 * 
 */
class SafeCaller final
{
public:
    using VoidFunc = std::function<void()>;

    /**
     * @brief Helper class which contains parent's address.
     * Is used for tracking if parent object is destroyed.
     * 
     */
    struct Guard {
        Guard(size_t callerAddress)
            : m_callerAddress("0x" + tools::to_hex_string(callerAddress))
        {
        }
        const std::string m_callerAddress;
    };

    /**
     * @brief Construct a new Safe Caller object
     * 
     * @param callerAddress address of parent
     */
    SafeCaller(size_t callerAddress)
        : m_caller(std::make_shared<Guard>(callerAddress))
    {
    }

    /**
     * @brief Construct a new Safe Caller object
     * 
     * @tparam T type of parent
     * @param t pointer to parent
     */
    template <typename T>
    SafeCaller(const T *const t)
        : m_caller(std::make_shared<Guard>(reinterpret_cast<size_t>(t)))
    {
    }

    /**
     * @brief Releases guard, so that all calls made after calling this method will not be executed.
     * 
     */
    void release() noexcept
    {
        if (m_caller) {
            m_caller.reset();
        }
    }

    /**
     * @brief Creates function with guarantee that function will not be executed if caller is destroyed.
     * 
     * @tparam Func type of function
     * @param fn function object
     * @param fallback function to be called if caller does not exist
     * @param cbName name of function
     * @return auto wrapped function
     */
    template <typename Func>
    auto invoke(Func &&fn, VoidFunc &&fallback = nullptr, const std::string &cbName = "")
    {
        std::weak_ptr<Guard> weakPtr = m_caller;
        return [weakPtr,
                addr = address(),
                fn = std::forward<Func>(fn),
                fallback = std::forward<VoidFunc>(fallback),
                cbName = std::move(cbName)](auto &&...args) {
            if (weakPtr.expired() || !weakPtr.lock()) {
                if (fallback) {
                    fallback();
                } else {
                    LOG_ERROR_STATIC("caller: " << addr << " " << cbName << " expired");
                }
                return;
            }

            fn(std::forward<decltype(args)>(args)...);
        };
    }

    /**
     * @brief Returns caller's address.
     * 
     * @return const std::string caller address
     */
    const std::string address() const
    {
        return m_caller ? m_caller->m_callerAddress : "[removed]";
    }

private:
    std::shared_ptr<Guard> m_caller;
};

} // namespace psi::comm
