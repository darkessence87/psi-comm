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

TEST(EventTests, event_outlives_subscription)
{
    Event<int> a;

    MockedFn<std::function<void(int)>> onEventFn;
    auto sub = a.subscribe(onEventFn.fn());
    EXPECT_NE(sub, nullptr);

    sub.reset();

    EXPECT_CALL(onEventFn, f(_)).Times(0);
    a.notify(20);
}

TEST(EventTests, subscription_outlives_event)
{
    auto a = std::make_shared<Event<int>>();

    MockedFn<std::function<void(int)>> onEventFn;
    auto sub = a->subscribe(onEventFn.fn());
    EXPECT_NE(sub, nullptr);

    a.reset();
    sub.reset();
}

TEST(EventTests, subscribe)
{
    Event<int> a;

    MockedFn<std::function<void(int)>> onEventFn;
    auto sub = a.subscribe(onEventFn.fn());
    EXPECT_NE(sub, nullptr);
}
