
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
    struct Locker {
        Locker(ObjectType *const obj, MutexType &mtx)
            : m_obj(obj)
            , m_lock(mtx)
        {
        }

        Locker(Locker &) = delete;
        Locker &operator=(Locker &) = delete;

        Locker(Locker &&locker)
            : m_obj(std::move(locker.m_obj))
            , m_lock(std::move(locker.m_lock))
        {
        }

        Locker &&operator=(Locker &&locker)
        {
            return std::move(locker);
        }

        ObjectType *operator->()
        {
            return m_obj;
        }

        const ObjectType *operator->() const
        {
            return m_obj;
        }

    private:
        ObjectType *const m_obj;
        std::unique_lock<MutexType> m_lock;
    };

    Synched(std::shared_ptr<ObjectType> obj)
        : m_object(obj)
        , m_mutex(std::make_shared<MutexType>())
    {
    }

    Locker operator->()
    {
        return Locker(m_object.get(), *m_mutex);
    }

    const Locker operator->() const
    {
        return Locker(m_object.get(), *m_mutex);
    }

private:
    std::shared_ptr<ObjectType> m_object;
    std::shared_ptr<MutexType> m_mutex;
};

} // namespace psi::comm