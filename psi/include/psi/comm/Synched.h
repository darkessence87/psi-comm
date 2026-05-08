
#pragma once

#include <mutex>

namespace psi::comm {

/**
 * @brief Synched class is used for thread-safe access to wrapped object.
 * 
 * @tparam ObjectType type of object
 * @tparam MutexType type of mutex
 */
template <typename ObjectType, typename MutexType = std::recursive_mutex>
class Synched
{
public:
    /**
     * @brief RAII lock guard that holds the mutex for the lifetime of the object.
     *
     * Returned by @c Synched::operator->().  While the Locker is alive the
     * mutex is held, giving exclusive access to the wrapped object.  The lock
     * is released when the Locker is destroyed (i.e. at the end of the
     * expression containing the @c -> call).
     */
    struct Locker {
        /**
         * @brief Acquire the mutex and wrap the object pointer.
         * @param obj  Raw pointer to the wrapped object.
         * @param mtx  Mutex to lock for the duration of this Locker's lifetime.
         */
        Locker(ObjectType *const obj, MutexType &mtx)
            : m_obj(obj)
            , m_lock(mtx)
        {
        }

        Locker(Locker &) = delete;
        Locker &operator=(Locker &) = delete;

        /// @brief Move constructor; transfers lock ownership.
        Locker(Locker &&locker)
            : m_obj(std::move(locker.m_obj))
            , m_lock(std::move(locker.m_lock))
        {
        }

        Locker &&operator=(Locker &&locker)
        {
            return std::move(locker);
        }

        /// @brief Access the wrapped object while holding the lock.
        ObjectType *operator->()
        {
            return m_obj;
        }

        /// @brief Access the wrapped object (const) while holding the lock.
        const ObjectType *operator->() const
        {
            return m_obj;
        }

    private:
        ObjectType *const m_obj;
        std::unique_lock<MutexType> m_lock;
    };

    /**
     * @brief Construct a Synched wrapper around an existing shared object.
     * @param obj Shared pointer to the object to protect.
     */
    Synched(std::shared_ptr<ObjectType> obj)
        : m_object(obj)
        , m_mutex(std::make_shared<MutexType>())
    {
    }

    /**
     * @brief Acquire the mutex and return a Locker granting access to the object.
     *
     * Typical usage: @code synched->method(); @endcode
     * The mutex is held until the Locker returned by this call is destroyed.
     *
     * @return Locker RAII guard with a pointer to the wrapped object.
     */
    Locker operator->()
    {
        return Locker(m_object.get(), *m_mutex);
    }

    /// @brief Const overload of operator->(); acquires the mutex in the same way.
    const Locker operator->() const
    {
        return Locker(m_object.get(), *m_mutex);
    }

private:
    std::shared_ptr<ObjectType> m_object;
    std::shared_ptr<MutexType> m_mutex;
};

} // namespace psi::comm