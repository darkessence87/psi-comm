#include "psi/test/psi_mock.h"

#include "psi/comm/Event.h"

using namespace psi::comm;
using namespace psi::test;

TEST(Event_Tests, notify)
{
    Event<int> a;

    auto onEventFn = MockedFn<std::function<void(int)>>::create();

    {
        // SCOPED_TRACE("// case 1. subscription is saved");

        auto sub = a.subscribe(onEventFn->fn());
        EXPECT_CALL(onEventFn, 1).WithArgs(20);
        a.notify(20);
    }

    {
        // SCOPED_TRACE("// case 2. subscription is not saved");

        a.subscribe(onEventFn->fn());
        EXPECT_CALL(onEventFn, 0);
        a.notify(20);
    }
}

TEST(Event_Tests, event_outlives_subscription)
{
    Event<int> a;

    auto onEventFn = MockedFn<std::function<void(int)>>::create();
    auto sub = a.subscribe(onEventFn->fn());
    EXPECT_NE(sub, nullptr);

    sub.reset();

    EXPECT_CALL(onEventFn, 0);
    a.notify(20);
}

TEST(Event_Tests, subscription_outlives_event)
{
    auto a = std::make_shared<Event<int>>();

    auto onEventFn = MockedFn<std::function<void(int)>>::create();
    auto sub = a->subscribe(onEventFn->fn());
    EXPECT_NE(sub, nullptr);

    a.reset();
    sub.reset();
}

TEST(Event_Tests, subscribe)
{
    Event<int> a;

    auto onEventFn = MockedFn<std::function<void(int)>>::create();
    auto sub = a.subscribe(onEventFn->fn());
    EXPECT_NE(sub, nullptr);
}
