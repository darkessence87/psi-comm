#include "TestHelper.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "psi/comm/Event.h"
#undef private

using namespace ::testing;
using namespace psi::comm;
using namespace psi::test;

TEST(EventTests, notify)
{
    Event<int> a;

    MockedFn<std::function<void(int)>> onEventFn;

    {
        SCOPED_TRACE("// case 1. subscription is saved");

        auto sub = a.subscribe(onEventFn.fn());
        EXPECT_CALL(onEventFn, f(20));
        a.notify(20);
    }

    {
        SCOPED_TRACE("// case 2. subscription is not saved");

        a.subscribe(onEventFn.fn());
        EXPECT_CALL(onEventFn, f(_)).Times(0);
        a.notify(20);
    }
}

TEST(EventTests, subscribe)
{
    Event<int> a;

    MockedFn<std::function<void(int)>> onEventFn;
    auto sub = a.subscribe(onEventFn.fn());
    EXPECT_NE(sub, nullptr);
}

TEST(EventTests, createListener)
{
    Event<int> a;

    auto listener = a.createListener();
    EXPECT_NE(listener, nullptr);

    a.notify(20);

    MockedFn<std::function<void(int)>> onEventFn;
    listener->setFunction(onEventFn.fn());

    EXPECT_CALL(onEventFn, f(25));
    a.notify(25);
}