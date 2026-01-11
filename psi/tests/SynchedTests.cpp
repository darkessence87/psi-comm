
#include <gtest/gtest.h>

#include "psi/comm/Synched.h"

using namespace ::testing;
using namespace psi::comm;

template <typename T>
void doTest(T a, const int threadsN)
{
    auto fnInc = [&a](const int N) {
        for (int i = 0; i < N; ++i) {
            a->increment();
        }
    };

    auto fnDec = [&a](const int N) {
        for (int i = 0; i < N; ++i) {
            a->decrement();
        }
    };

    const int operationsN = 10'000'000;
    const int operationsPerThread = operationsN / threadsN;

    std::vector<std::thread> threads;
    for (int i = 0; i < threadsN; ++i) {
        if (i % 2) {
            threads.emplace_back(std::thread(fnInc, operationsPerThread));
        } else {
            threads.emplace_back(std::thread(fnDec, operationsPerThread));
        }
    }

    for (auto &t : threads) {
        t.join();
    }

    EXPECT_EQ(a->getValue(), 0);
}

struct A {
    virtual ~A() = default;
    virtual void increment()
    {
        ++m_value;
    }
    virtual void decrement()
    {
        --m_value;
    }
    virtual int getValue()
    {
        return m_value;
    }

protected:
    int m_value = 0;
};

struct SpinLockedA : A {
    SpinLockedA()
        : A()
    {
    }

    void increment() override
    {
        while (lock.test_and_set(std::memory_order_acquire))
            ;
        ++m_value;
        lock.clear(std::memory_order_release);
    }

    void decrement() override
    {
        while (lock.test_and_set(std::memory_order_acquire))
            ;
        --m_value;
        lock.clear(std::memory_order_release);
    }

    SpinLockedA *operator->()
    {
        return this;
    }
    const SpinLockedA *operator->() const
    {
        return this;
    }

    std::atomic_flag lock = ATOMIC_FLAG_INIT;
};

struct MutexLockedA {
    MutexLockedA()
        : a(std::make_shared<A>())
    {
    }
    void increment()
    {
        a->increment();
    }
    void decrement()
    {
        a->decrement();
    }
    int getValue()
    {
        return a->getValue();
    }
    MutexLockedA *operator->()
    {
        return this;
    }
    const MutexLockedA *operator->() const
    {
        return this;
    }
    Synched<A> a;
};

struct AtomicA {
    AtomicA()
        : m_value(0)
    {
    }
    void increment()
    {
        ++m_value;
    }
    void decrement()
    {
        --m_value;
    }
    int getValue()
    {
        return m_value;
    }
    AtomicA *operator->()
    {
        return this;
    }
    const AtomicA *operator->() const
    {
        return this;
    }

protected:
    std::atomic<int> m_value;
};

struct SynchedTest : public TestWithParam<int> {
};

TEST_P(SynchedTest, MultiThread_SpinLock)
{
    doTest(SpinLockedA {}, GetParam());
}

TEST_P(SynchedTest, MultiThread_MutexLock)
{
    doTest(MutexLockedA {}, GetParam());
}

TEST_P(SynchedTest, MultiThread_Atomic)
{
    doTest(AtomicA {}, GetParam());
}

const int testParams[] = {2, 4, 6, 8, 16};

INSTANTIATE_TEST_SUITE_P(SynchedTests, SynchedTest, ValuesIn(testParams));