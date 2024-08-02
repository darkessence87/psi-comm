#include "TestHelper.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "psi/comm/Attribute.h"
#undef private

using namespace ::testing;
using namespace psi::comm;
using namespace psi::test;

TEST(AttributeTests, ctor)
{
    {
        SCOPED_TRACE("// case 1. default value");

        struct A {
            int m_data = 15;
        };

        Attribute<A> a;
        EXPECT_EQ(a.m_value.m_data, 15);
    }

    {
        SCOPED_TRACE("// case 2. non-default value");

        Attribute<int> a(15);
        EXPECT_EQ(a.m_value, 15);
    }
}

TEST(AttributeTests, value)
{
    Attribute<int> a(15);
    EXPECT_EQ(a.value(), 15);
}

TEST(AttributeTests, setValue)
{
    Attribute<int> a(15);

    MockedFn<std::function<void(int, int)>> onEventFn;
    auto sub = a.m_event->subscribe(onEventFn.fn());

    {
        SCOPED_TRACE("// case 1. data is changed");

        EXPECT_CALL(onEventFn, f(15, 20));
        a.setValue(20);

        EXPECT_EQ(a.value(), 20);
    }

    {
        SCOPED_TRACE("// case 2. data is not changed");

        EXPECT_CALL(onEventFn, f(_, _)).Times(0);
        a.setValue(20);
    }
}

TEST(AttributeTests, subscribe)
{
    Attribute<int> a(15);

    MockedFn<std::function<void(int, int)>> onEventFn;
    auto sub = a.subscribe(onEventFn.fn());
    EXPECT_NE(sub, nullptr);
}

TEST(AttributeTests, subscribeAndGet)
{
    Attribute<int> a(15);

    MockedFn<std::function<void(int, int)>> onEventFn;

    EXPECT_CALL(onEventFn, f(15, 15));
    auto sub = a.subscribeAndGet(onEventFn.fn());
    EXPECT_NE(sub, nullptr);
}
