#include "psi/test/psi_mock.h"

#include "psi/comm/Attribute.h"

using namespace psi::comm;
using namespace psi::test;

template <typename T>
struct Attribute<T>::Attribute_Tests {
    static auto value(Attribute<T> &a)
    {
        return a.m_value;
    }
    static auto event(Attribute<T> &a)
    {
        return a.m_event.get();
    }
};

TEST(Attribute_Tests, ctor)
{
    {
        // SCOPED_TRACE("// case 1. default value");

        struct A {
            int m_data = 15;
        };

        Attribute<A> a;
        EXPECT_EQ(Attribute<A>::Attribute_Tests::value(a).m_data, 15);
    }

    {
        // SCOPED_TRACE("// case 2. non-default value");

        Attribute<int> a(15);
        EXPECT_EQ(Attribute<int>::Attribute_Tests::value(a), 15);
    }
}

TEST(Attribute_Tests, value)
{
    Attribute<int> a(15);
    EXPECT_EQ(a.value(), 15);
}

TEST(Attribute_Tests, setValue)
{
    Attribute<int> a(15);

    auto onEventFn = MockedFn<std::function<void(int,int)>>::create();
    auto sub = Attribute<int>::Attribute_Tests::event(a)->subscribe(onEventFn->fn());

    {
        // SCOPED_TRACE("// case 1. data is changed");

        EXPECT_CALL(onEventFn, 1).WithArgs(15, 20);
        a.setValue(20);

        EXPECT_EQ(a.value(), 20);
    }

    TestLib::verify_and_clear_expectations();

    {
        // SCOPED_TRACE("// case 2. data is not changed");

        EXPECT_CALL(onEventFn, 0);
        a.setValue(20);
    }
}

TEST(Attribute_Tests, subscribe)
{
    Attribute<int> a(15);

    MockedFn<std::function<void(int, int)>> onEventFn;
    auto sub = a.subscribe(onEventFn.fn());
    EXPECT_NE(sub, nullptr);
}

TEST(Attribute_Tests, subscribeAndGet)
{
    Attribute<int> a(15);

    auto onEventFn = MockedFn<std::function<void(int,int)>>::create();

    EXPECT_CALL(onEventFn, 1).WithArgs(15, 15);
    auto sub = a.subscribeAndGet(onEventFn->fn());
    EXPECT_NE(sub, nullptr);
}
