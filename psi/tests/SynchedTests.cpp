#include "psi/test/psi_mock.h"

#include "psi/comm/Synched.h"

#include <atomic>
#include <thread>
#include <vector>

using namespace psi::comm;
using namespace psi::test;

// Runs threadsN threads alternating increment/decrement (10 000 000 operations
// total), then asserts the counter returned to 0.
template <typename T>
static void doTest(T a, const int threadsN)
{
    const int operationsN = 10'000'000;
    const int operationsPerThread = operationsN / threadsN;

    auto fnInc = [&a](const int N) {
        for (int i = 0; i < N; ++i) a->increment();
    };
    auto fnDec = [&a](const int N) {
        for (int i = 0; i < N; ++i) a->decrement();
    };

    std::vector<std::thread> threads;
    threads.reserve(threadsN);
    for (int i = 0; i < threadsN; ++i) {
        if (i % 2) {
            threads.emplace_back(fnInc, operationsPerThread);
        } else {
            threads.emplace_back(fnDec, operationsPerThread);
        }
    }
    for (auto &t : threads) t.join();

    EXPECT_EQ(a->getValue(), 0);
}

// ---------------------------------------------------------------------------
// Helpers: thread-safe wrappers for the plain integer counter struct A.
// ---------------------------------------------------------------------------

struct A {
    virtual ~A() = default;
    virtual void increment() { ++m_value; }
    virtual void decrement() { --m_value; }
    virtual int getValue() { return m_value; }

protected:
    int m_value = 0;
};

// Spin-lock protected via std::atomic_flag.
struct SpinLockedA : A {
    SpinLockedA() : A() {}

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

    SpinLockedA *operator->() { return this; }
    const SpinLockedA *operator->() const { return this; }

    std::atomic_flag lock = ATOMIC_FLAG_INIT;
};

// Mutex-locked via Synched<A> (psi::comm wrapper).
struct MutexLockedA {
    MutexLockedA() : a(std::make_shared<A>()) {}

    void increment() { a->increment(); }
    void decrement() { a->decrement(); }
    int getValue() { return a->getValue(); }

    MutexLockedA *operator->() { return this; }
    const MutexLockedA *operator->() const { return this; }

    Synched<A> a;
};

// Lock-free via std::atomic<int>.
struct AtomicA {
    AtomicA() : m_value(0) {}

    void increment() { ++m_value; }
    void decrement() { --m_value; }
    int getValue() { return m_value; }

    AtomicA *operator->() { return this; }
    const AtomicA *operator->() const { return this; }

protected:
    std::atomic<int> m_value;
};

// ---------------------------------------------------------------------------
// Tests: run each synchronisation strategy with thread-counts {2,4,6,8,16}.
// ---------------------------------------------------------------------------

TEST(SynchedTests, MultiThread_SpinLock)
{
    for (int n : {2, 4, 6, 8, 16}) {
        doTest(SpinLockedA {}, n);
    }
}

TEST(SynchedTests, MultiThread_MutexLock)
{
    for (int n : {2, 4, 6, 8, 16}) {
        doTest(MutexLockedA {}, n);
    }
}

TEST(SynchedTests, MultiThread_Atomic)
{
    for (int n : {2, 4, 6, 8, 16}) {
        doTest(AtomicA {}, n);
    }
}
