#include "psi/test/psi_mock.h"

#include <array>
#include <functional>

#include "psi/comm/call_strategy/cb/AsyncCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/CachedCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/FullySyncCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/PartlySuppressedCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/SuppressedCbStrategy.hpp"

#include "psi/comm/call_strategy/ev/CachedEvStrategy.hpp"
#include "psi/comm/call_strategy/ev/FullySyncEvStrategy.hpp"
#include "psi/comm/call_strategy/ev/SuppressedEvStrategy.hpp"

using namespace psi::comm;
using namespace psi::test;

// ----------------------------------------------------------------------------
// AsyncCbStrategy
// All requests are dispatched immediately; responses are independent.
// ----------------------------------------------------------------------------
TEST(CallStrategyTests, AsyncCbStrategy)
{
    AsyncCbStrategy<bool, int> st;

    using Response = std::function<void(bool, int)>;

    Response tmpA, tmpB, tmpC;
    int reqCallsA = 0, reqCallsB = 0, reqCallsC = 0;

    auto resA = MockedFn<Response>::create();
    auto resB = MockedFn<Response>::create();
    auto resC = MockedFn<Response>::create();

    // Async: all three requests are dispatched immediately.
    st.processRequest([&](Response cb) { ++reqCallsA; tmpA = cb; }, resA->fn());
    st.processRequest([&](Response cb) { ++reqCallsB; tmpB = cb; }, resB->fn());
    st.processRequest([&](Response cb) { ++reqCallsC; tmpC = cb; }, resC->fn());

    EXPECT_EQ(reqCallsA, 1);
    EXPECT_EQ(reqCallsB, 1);
    EXPECT_EQ(reqCallsC, 1);

    EXPECT_CALL(resC, 1).WithArgs(true, 3);
    EXPECT_CALL(resB, 1).WithArgs(false, 2);
    EXPECT_CALL(resA, 1).WithArgs(true, 1);

    tmpC(true, 3);
    tmpB(false, 2);
    tmpA(true, 1);
}

// ----------------------------------------------------------------------------
// CachedCbStrategy
// Caches results by input key; duplicate keys share a single in-flight request.
// ----------------------------------------------------------------------------
TEST(CallStrategyTests, CachedCbStrategy_DifferentKeys)
{
    // Case 1: different keys → each request is dispatched independently (async).
    using Response = std::function<void()>;
    const uint8_t N = 5;

    CachedCbStrategy<int> st;

    std::array<std::function<void()>, N> tmp;
    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    for (uint8_t i = 0; i < N; ++i) {
        st.processRequest(i + 1, [&, i](Response cb) { ++reqCalls[i]; tmp[i] = cb; }, res[i]->fn());
        EXPECT_EQ(reqCalls[i], 1); // each key dispatched immediately
    }

    for (uint8_t i = 0; i < N; ++i) {
        EXPECT_CALL(res[i], 1);
        tmp[i]();
        TestLib::verify_and_clear_expectations();
    }
}

TEST(CallStrategyTests, CachedCbStrategy_SameKeyBeforeResponse)
{
    // Case 2: same key, all queued before first response fires.
    // Only the first request is dispatched; all callbacks share the result.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    CachedCbStrategy<int> st;

    std::function<void()> tmp;
    int reqCalls0 = 0;
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    // First request: dispatched, captures internal callback.
    st.processRequest(1, [&](Response cb) { ++reqCalls0; tmp = cb; }, res[0]->fn());
    EXPECT_EQ(reqCalls0, 1);

    // Remaining: same key, cache miss with pending result → add callback only.
    for (uint8_t i = 1; i < N; ++i) {
        st.processRequest(1, [](Response) {}, res[i]->fn());
    }

    // Fire response → all N callbacks invoked.
    for (uint8_t i = 0; i < N; ++i) {
        EXPECT_CALL(res[i], 1);
    }
    tmp();
}

TEST(CallStrategyTests, CachedCbStrategy_SameKeyAfterResponse)
{
    // Case 3: same key, first request completes before remaining arrive.
    // Subsequent requests with the same key get the cached result immediately.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    CachedCbStrategy<int> st;

    std::function<void()> tmp;
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    // First request: complete immediately.
    st.processRequest(1, [&](Response cb) { tmp = cb; }, res[0]->fn());
    EXPECT_CALL(res[0], 1);
    tmp();
    TestLib::verify_and_clear_expectations();

    // Remaining: cache hit → res called immediately inside processRequest.
    for (uint8_t i = 1; i < N; ++i) {
        EXPECT_CALL(res[i], 1);
        st.processRequest(1, [](Response) {}, res[i]->fn());
        TestLib::verify_and_clear_expectations();
    }
}

// ----------------------------------------------------------------------------
// FullySyncCbStrategy
// Requests are processed one at a time, chained via callback.
// ----------------------------------------------------------------------------
TEST(CallStrategyTests, FullySyncCbStrategy_RequestsBeforeResponse)
{
    // Case 1: queue all N requests first; responses chain one by one.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    FullySyncCbStrategy<> st;

    std::array<std::function<void()>, N> tmp;
    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    for (uint8_t i = 0; i < N; ++i) {
        st.processRequest([&, i](Response cb) { ++reqCalls[i]; tmp[i] = cb; }, res[i]->fn());
    }

    // Only first request dispatched.
    EXPECT_EQ(reqCalls[0], 1);
    for (uint8_t i = 1; i < N; ++i) EXPECT_EQ(reqCalls[i], 0);

    // Fire each callback: response[i] is called, then request[i+1] is dispatched.
    for (uint8_t i = 0; i < N; ++i) {
        EXPECT_CALL(res[i], 1);
        tmp[i]();
        TestLib::verify_and_clear_expectations();
        if (i < N - 1) EXPECT_EQ(reqCalls[i + 1], 1);
    }
}

TEST(CallStrategyTests, FullySyncCbStrategy_RequestsAfterResponse)
{
    // Case 2: each request is submitted and its callback fired immediately.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    FullySyncCbStrategy<> st;

    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    for (uint8_t i = 0; i < N; ++i) {
        std::function<void()> tmp;
        st.processRequest([&, i](Response cb) { ++reqCalls[i]; tmp = cb; }, res[i]->fn());
        EXPECT_EQ(reqCalls[i], 1);
        EXPECT_CALL(res[i], 1);
        tmp();
        TestLib::verify_and_clear_expectations();
    }
}

// ----------------------------------------------------------------------------
// PartlySuppressedCbStrategy
// First request sent; remaining queued. On response: call all but last callback,
// then send the last request. On its response: call last callback.
// ----------------------------------------------------------------------------
TEST(CallStrategyTests, PartlySuppressedCbStrategy_RequestsBeforeResponse)
{
    // Case 1: queue all N, fire first callback → res[0..N-2] called, req[N-1] dispatched.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    PartlySuppressedCbStrategy<> st;

    std::array<std::function<void()>, N> tmp;
    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    for (uint8_t i = 0; i < N; ++i) {
        st.processRequest([&, i](Response cb) { ++reqCalls[i]; tmp[i] = cb; }, res[i]->fn());
    }

    EXPECT_EQ(reqCalls[0], 1);
    for (uint8_t i = 1; i < N; ++i) EXPECT_EQ(reqCalls[i], 0);

    // Fire first callback: res[0..N-2] called, req[N-1] dispatched.
    for (uint8_t i = 0; i < N - 1; ++i) EXPECT_CALL(res[i], 1);
    tmp[0]();
    TestLib::verify_and_clear_expectations();

    EXPECT_EQ(reqCalls[N - 1], 1);

    // Fire last callback: res[N-1] called.
    EXPECT_CALL(res[N - 1], 1);
    tmp[N - 1]();
}

TEST(CallStrategyTests, PartlySuppressedCbStrategy_RequestsAfterResponse)
{
    // Case 2: each request submitted and its callback fired immediately.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    PartlySuppressedCbStrategy<> st;

    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    for (uint8_t i = 0; i < N; ++i) {
        std::function<void()> tmp;
        st.processRequest([&, i](Response cb) { ++reqCalls[i]; tmp = cb; }, res[i]->fn());
        EXPECT_EQ(reqCalls[i], 1);
        EXPECT_CALL(res[i], 1);
        tmp();
        TestLib::verify_and_clear_expectations();
    }
}

// ----------------------------------------------------------------------------
// SuppressedCbStrategy
// First request sent; remaining queued. On response: all callbacks receive
// the same result values and all pending requests are suppressed.
// ----------------------------------------------------------------------------
TEST(CallStrategyTests, SuppressedCbStrategy_RequestsBeforeResponse)
{
    // Case 1: queue all N, fire first callback → all N responses called.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    SuppressedCbStrategy<> st;

    std::function<void()> tmp;
    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    // First call: dispatched immediately, captures the while-loop callback.
    st.processRequest([&](Response cb) { ++reqCalls[0]; tmp = cb; }, res[0]->fn());
    EXPECT_EQ(reqCalls[0], 1);

    // Remaining: queued without dispatching.
    for (uint8_t i = 1; i < N; ++i) {
        st.processRequest([&, i](Response cb) { ++reqCalls[i]; }, res[i]->fn());
        EXPECT_EQ(reqCalls[i], 0);
    }

    // Fire: all N responses called via while-loop.
    for (uint8_t i = 0; i < N; ++i) EXPECT_CALL(res[i], 1);
    tmp();
}

TEST(CallStrategyTests, SuppressedCbStrategy_RequestsAfterResponse)
{
    // Case 2: each request submitted and its callback fired immediately.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    SuppressedCbStrategy<> st;

    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    for (uint8_t i = 0; i < N; ++i) {
        std::function<void()> tmp;
        st.processRequest([&, i](Response cb) { ++reqCalls[i]; tmp = cb; }, res[i]->fn());
        EXPECT_EQ(reqCalls[i], 1);
        EXPECT_CALL(res[i], 1);
        tmp();
        TestLib::verify_and_clear_expectations();
    }
}

// ----------------------------------------------------------------------------
// FullySyncEvStrategy
// Queue requests, send one at a time. processEvent() advances to the next.
// ----------------------------------------------------------------------------
TEST(CallStrategyTests, FullySyncEvStrategy_RequestsBeforeResponse)
{
    // Case 1: queue all N, then processEvent() one by one.
    using OnEvent = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    const uint8_t N = 5;

    FullySyncEvStrategy<TypeList<>, TypeList<>> st;
    ValidationFn valFn = [] { return true; };

    std::array<std::function<bool()>, N> tmp; // validation callbacks
    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<OnEvent>>, N> res;
    for (auto &r : res) r = MockedFn<OnEvent>::create();

    for (uint8_t i = 0; i < N; ++i) {
        st.processRequest(
            [&, i](std::function<bool()> resp) { ++reqCalls[i]; tmp[i] = resp; },
            std::ref(valFn),
            res[i]->fn());
    }

    // Only first request dispatched.
    EXPECT_EQ(reqCalls[0], 1);
    for (uint8_t i = 1; i < N; ++i) EXPECT_EQ(reqCalls[i], 0);

    for (uint8_t i = 0; i < N; ++i) {
        // Validate: signal "request accepted", strategy waits for event.
        tmp[i]();
        EXPECT_CALL(res[i], 1);
        st.processEvent(); // fires onEvent[i], dispatches req[i+1] if any
        TestLib::verify_and_clear_expectations();
        if (i < N - 1) EXPECT_EQ(reqCalls[i + 1], 1);
    }
}

TEST(CallStrategyTests, FullySyncEvStrategy_RequestsAfterResponse)
{
    // Case 2: submit one request at a time, validate and fire event immediately.
    using OnEvent = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    const uint8_t N = 5;

    FullySyncEvStrategy<TypeList<>, TypeList<>> st;
    ValidationFn valFn = [] { return true; };

    std::array<std::shared_ptr<MockedFn<OnEvent>>, N> res;
    for (auto &r : res) r = MockedFn<OnEvent>::create();

    for (uint8_t i = 0; i < N; ++i) {
        std::function<bool()> tmp;
        st.processRequest(
            [&, i](std::function<bool()> resp) { tmp = resp; },
            std::ref(valFn),
            res[i]->fn());
        tmp();
        EXPECT_CALL(res[i], 1);
        st.processEvent();
        TestLib::verify_and_clear_expectations();
    }
}

// ----------------------------------------------------------------------------
// SuppressedEvStrategy
// First request sent; remaining queued. processEvent() fans out all onEvent
// callbacks with the same event arguments.
// ----------------------------------------------------------------------------
TEST(CallStrategyTests, SuppressedEvStrategy_RequestsBeforeResponse)
{
    // Case 1: queue all N, one processEvent() fans out all N onEvent callbacks.
    using OnEvent = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    const uint8_t N = 5;

    SuppressedEvStrategy<TypeList<>, TypeList<>> st;
    ValidationFn valFn = [] { return true; };

    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<OnEvent>>, N> res;
    for (auto &r : res) r = MockedFn<OnEvent>::create();

    // All N requests queued; req[0] validated immediately.
    for (uint8_t i = 0; i < N; ++i) {
        st.processRequest(
            [&, i](std::function<bool()> resp) {
                ++reqCalls[i];
                resp(); // validate immediately
            },
            std::ref(valFn),
            res[i]->fn());
    }

    EXPECT_EQ(reqCalls[0], 1);
    for (uint8_t i = 1; i < N; ++i) EXPECT_EQ(reqCalls[i], 0);

    for (uint8_t i = 0; i < N; ++i) EXPECT_CALL(res[i], 1);
    st.processEvent();
}

TEST(CallStrategyTests, SuppressedEvStrategy_RequestsAfterResponse)
{
    // Case 2: each request submitted, validated, and event fired immediately.
    using OnEvent = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    const uint8_t N = 5;

    SuppressedEvStrategy<TypeList<>, TypeList<>> st;
    ValidationFn valFn = [] { return true; };

    std::array<std::shared_ptr<MockedFn<OnEvent>>, N> res;
    for (auto &r : res) r = MockedFn<OnEvent>::create();

    for (uint8_t i = 0; i < N; ++i) {
        st.processRequest(
            [&](std::function<bool()> resp) { resp(); },
            std::ref(valFn),
            res[i]->fn());
        EXPECT_CALL(res[i], 1);
        st.processEvent();
        TestLib::verify_and_clear_expectations();
    }
}

// ----------------------------------------------------------------------------
// CachedEvStrategy
// Caches event results by input key; shares in-flight requests for duplicate keys.
// ----------------------------------------------------------------------------
TEST(CallStrategyTests, CachedEvStrategy_DifferentKeys)
{
    // Case 1: different keys → each goes through the underlying FullySyncEvStrategy.
    // Requests are chained one at a time; each processEvent() advances the queue.
    using OnEvent = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    const uint8_t N = 5;

    CachedEvStrategy<int, TypeList<>, TypeList<>> st;
    ValidationFn valFn = [] { return true; };

    std::array<std::function<bool()>, N> tmp; // validation callbacks
    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<OnEvent>>, N> res;
    for (auto &r : res) r = MockedFn<OnEvent>::create();

    for (uint8_t i = 0; i < N; ++i) {
        st.processRequest(
            i + 1,
            [&, i](std::function<bool()> resp) { ++reqCalls[i]; tmp[i] = resp; },
            std::ref(valFn),
            res[i]->fn());
    }

    // Underlying FullySyncEvStrategy: only req[0] dispatched.
    EXPECT_EQ(reqCalls[0], 1);
    for (uint8_t i = 1; i < N; ++i) EXPECT_EQ(reqCalls[i], 0);

    for (uint8_t i = 0; i < N; ++i) {
        tmp[i](); // validate → waiting for event
        EXPECT_CALL(res[i], 1);
        st.processEvent();
        TestLib::verify_and_clear_expectations();
        if (i < N - 1) EXPECT_EQ(reqCalls[i + 1], 1);
    }
}

TEST(CallStrategyTests, CachedEvStrategy_SameKeyBeforeResponse)
{
    // Case 2: same key, all queued before first event fires.
    // Only first request dispatched; all onEvent callbacks share the result.
    using OnEvent = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    const uint8_t N = 5;

    CachedEvStrategy<int, TypeList<>, TypeList<>> st;
    ValidationFn valFn = [] { return true; };

    std::array<std::shared_ptr<MockedFn<OnEvent>>, N> res;
    for (auto &r : res) r = MockedFn<OnEvent>::create();

    // First request: validate immediately so strategy waits for event.
    int reqCalls0 = 0;
    st.processRequest(
        1,
        [&](std::function<bool()> resp) { ++reqCalls0; resp(); },
        std::ref(valFn),
        res[0]->fn());
    EXPECT_EQ(reqCalls0, 1);

    // Remaining: same key, no result yet → add callbacks only.
    for (uint8_t i = 1; i < N; ++i) {
        st.processRequest(1, [](std::function<bool()>) {}, std::ref(valFn), res[i]->fn());
    }

    // One event fires all N onEvent callbacks.
    for (uint8_t i = 0; i < N; ++i) EXPECT_CALL(res[i], 1);
    st.processEvent();
}

TEST(CallStrategyTests, CachedEvStrategy_SameKeyAfterResponse)
{
    // Case 3: same key, first request completes before remaining arrive.
    // Subsequent requests with same key get onEvent called immediately from cache.
    using OnEvent = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    const uint8_t N = 5;

    CachedEvStrategy<int, TypeList<>, TypeList<>> st;
    ValidationFn valFn = [] { return true; };

    std::array<std::shared_ptr<MockedFn<OnEvent>>, N> res;
    for (auto &r : res) r = MockedFn<OnEvent>::create();

    // First request: validate and fire event immediately.
    st.processRequest(
        1,
        [&](std::function<bool()> resp) { resp(); },
        std::ref(valFn),
        res[0]->fn());
    EXPECT_CALL(res[0], 1);
    st.processEvent();
    TestLib::verify_and_clear_expectations();

    // Remaining: cache hit → onEvent called immediately in processRequest.
    for (uint8_t i = 1; i < N; ++i) {
        EXPECT_CALL(res[i], 1);
        st.processRequest(1, [](std::function<bool()>) {}, std::ref(valFn), res[i]->fn());
        TestLib::verify_and_clear_expectations();
    }
}

// ----------------------------------------------------------------------------
// interrupt / interruptImmediately
// ----------------------------------------------------------------------------
TEST(CallStrategyTests, interrupt)
{
    // interrupt() flushes all pending response callbacks with default values.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    FullySyncCbStrategy<> st;

    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    // Only req[0] dispatched; rest queued.
    st.processRequest([&](Response cb) { ++reqCalls[0]; }, res[0]->fn());
    for (uint8_t i = 1; i < N; ++i) {
        st.processRequest([&, i](Response cb) { ++reqCalls[i]; }, res[i]->fn());
        EXPECT_EQ(reqCalls[i], 0);
    }
    EXPECT_EQ(reqCalls[0], 1);

    // interrupt() calls all pending responses.
    for (uint8_t i = 0; i < N; ++i) EXPECT_CALL(res[i], 1);
    st.interrupt();
}

TEST(CallStrategyTests, interruptImmediately)
{
    // interruptImmediately() drops all pending responses without calling them.
    using Response = std::function<void()>;
    const uint8_t N = 5;

    FullySyncCbStrategy<> st;

    std::array<int, N> reqCalls {};
    std::array<std::shared_ptr<MockedFn<Response>>, N> res;
    for (auto &r : res) r = MockedFn<Response>::create();

    // Only req[0] dispatched; rest queued.
    st.processRequest([&](Response cb) { ++reqCalls[0]; }, res[0]->fn());
    for (uint8_t i = 1; i < N; ++i) {
        st.processRequest([&, i](Response cb) { ++reqCalls[i]; }, res[i]->fn());
    }
    EXPECT_EQ(reqCalls[0], 1);

    // interruptImmediately() discards all — no response callbacks fired.
    for (uint8_t i = 0; i < N; ++i) EXPECT_CALL(res[i], 0);
    st.interruptImmediately();
}
