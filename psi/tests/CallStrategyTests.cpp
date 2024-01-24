#include "TestHelper.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "psi/comm/call_strategy/cb/AsyncCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/CachedCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/FullySyncCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/PartlySuppressedCbStrategy.hpp"
#include "psi/comm/call_strategy/cb/SuppressedCbStrategy.hpp"

#include "psi/comm/call_strategy/ev/CachedEvStrategy.hpp"
#include "psi/comm/call_strategy/ev/FullySyncEvStrategy.hpp"
#include "psi/comm/call_strategy/ev/SuppressedEvStrategy.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

using namespace ::testing;
using namespace psi::comm;
using namespace psi::test;

TEST(CallStrategyTests, AsyncCbStrategy)
{
    AsyncCbStrategy<bool, int> st;

    using Response = std::function<void(bool, int)>;
    using Request = std::function<void(int, Response)>;
    StrictMock<MockedFn<Request>> reqA, reqB, reqC;
    StrictMock<MockedFn<Response>> resA, resB, resC;
    Response tmpA, tmpB, tmpC;

    ::InSequence dummy;

    EXPECT_CALL(reqA, f(1, _)).WillOnce(SaveArg<1>(&tmpA));
    EXPECT_CALL(reqB, f(2, _)).WillOnce(SaveArg<1>(&tmpB));
    EXPECT_CALL(reqC, f(3, _)).WillOnce(SaveArg<1>(&tmpC));
    st.processRequest([req = reqA.fn()](auto resp) { req(1, resp); }, resA.fn());
    st.processRequest([req = reqB.fn()](auto resp) { req(2, resp); }, resB.fn());
    st.processRequest([req = reqC.fn()](auto resp) { req(3, resp); }, resC.fn());

    EXPECT_CALL(resC, f(true, 3));
    EXPECT_CALL(resB, f(false, 2));
    EXPECT_CALL(resA, f(true, 1));
    tmpC(true, 3);
    tmpB(false, 2);
    tmpA(true, 1);
}

TEST(CallStrategyTests, CachedCbStrategy)
{
    using Response = std::function<void()>;
    using Request = std::function<void(int, Response)>;
    const uint8_t N = 5;

    {
        SCOPED_TRACE("case 1. different requests by hash (Input)");

        CachedCbStrategy<int> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        Response tmp[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(SaveArg<1>(&tmp[i]));
            st.processRequest(
                i + 1, [rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
        }

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(res[i], f());
            tmp[i]();
        }
    }

    {
        SCOPED_TRACE("case 2. equal requests by hash before response (Input)");

        CachedCbStrategy<int> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        Response tmp[N];

        ::InSequence dummy;

        EXPECT_CALL(req[0], f(1, _)).WillOnce(SaveArg<1>(&tmp[0]));
        for (uint8_t i = 0; i < N; ++i) {
            st.processRequest(
                1, [rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
        }

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(res[i], f());
        }
        tmp[0]();
    }

    {
        SCOPED_TRACE("case 3. equal requests by hash after response (Input)");

        CachedCbStrategy<int> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        Response tmp[N];

        ::InSequence dummy;

        EXPECT_CALL(req[0], f(1, _)).WillOnce(SaveArg<1>(&tmp[0]));
        st.processRequest(
            1, [req = req[0].fn()](auto resp) { req(1, resp); }, res[0].fn());
        EXPECT_CALL(res[0], f());
        tmp[0]();

        for (uint8_t i = 1; i < N; ++i) {
            EXPECT_CALL(res[i], f());
            st.processRequest(
                1, [rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
        }
    }
}

TEST(CallStrategyTests, FullySyncCbStrategy)
{
    using Response = std::function<void()>;
    using Request = std::function<void(int, Response)>;
    const uint8_t N = 5;

    {
        SCOPED_TRACE("case 1. requests before response (Input)");

        FullySyncCbStrategy<> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        Response tmp[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            if (i == 0) {
                EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(SaveArg<1>(&tmp[i]));
            }
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
        }

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(res[i], f());
            if (i < N - 1) {
                EXPECT_CALL(req[i + 1], f(i + 2, _)).WillOnce(SaveArg<1>(&tmp[i + 1]));
            }
            tmp[i]();
        }
    }

    {
        SCOPED_TRACE("case 2. requests after response (Input)");

        FullySyncCbStrategy<> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        Response tmp[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(SaveArg<1>(&tmp[i]));
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
            EXPECT_CALL(res[i], f());
            tmp[i]();
        }
    }
}

TEST(CallStrategyTests, PartlySuppressedCbStrategy)
{
    using Response = std::function<void()>;
    using Request = std::function<void(int, Response)>;
    const uint8_t N = 5;

    {
        SCOPED_TRACE("case 1. requests before response (Input)");

        PartlySuppressedCbStrategy<> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        Response tmp[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            if (i == 0) {
                EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(SaveArg<1>(&tmp[i]));
            }
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
        }

        for (uint8_t i = 0; i < N; ++i) {
            if (i < N - 1) {
                EXPECT_CALL(res[i], f());
            } else {
                EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(SaveArg<1>(&tmp[i]));
            }
        }
        tmp[0]();

        EXPECT_CALL(res[N - 1], f());
        tmp[N - 1]();
    }

    {
        SCOPED_TRACE("case 2. requests after response (Input)");

        PartlySuppressedCbStrategy<> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        Response tmp[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(SaveArg<1>(&tmp[i]));
            EXPECT_CALL(res[i], f());
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
            tmp[i]();
        }
    }
}

TEST(CallStrategyTests, SuppressedCbStrategy)
{
    using Response = std::function<void()>;
    using Request = std::function<void(int, Response)>;
    const uint8_t N = 5;

    {
        SCOPED_TRACE("case 1. requests before response (Input)");

        SuppressedCbStrategy<> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        Response tmp[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            if (i == 0) {
                EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(SaveArg<1>(&tmp[i]));
            }
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
        }

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(res[i], f());
        }
        tmp[0]();
    }

    {
        SCOPED_TRACE("case 2. requests after response (Input)");

        SuppressedCbStrategy<> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        Response tmp[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(SaveArg<1>(&tmp[i]));
            EXPECT_CALL(res[i], f());
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
            tmp[i]();
        }
    }
}

TEST(CallStrategyTests, CachedEvStrategy)
{
    using Response = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    using Request = std::function<void(int, ValidationFn)>;
    const uint8_t N = 5;
    ValidationFn valFn = []() { return true; };

    {
        SCOPED_TRACE("case 1. different requests by hash (Input)");

        CachedEvStrategy<int, TypeList<>, TypeList<>> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];
        ValidationFn tmp[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            if (i == 0) {
                EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(SaveArg<1>(&tmp[i]));
            }
            st.processRequest(
                i + 1, [rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, std::ref(valFn), res[i].fn());
        }

        for (uint8_t i = 0; i < N; ++i) {
            tmp[i]();
            EXPECT_CALL(res[i], f());
            if (i < N - 1) {
                EXPECT_CALL(req[i + 1], f(i + 2, _)).WillOnce(SaveArg<1>(&tmp[i + 1]));
            }
            st.processEvent();
        }
    }

    {
        SCOPED_TRACE("case 2. equal requests by hash before response (Input)");

        CachedEvStrategy<int, TypeList<>, TypeList<>> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            if (i == 0) {
                EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(InvokeArgument<1>());
            }
            st.processRequest(
                1, [rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, std::ref(valFn), res[i].fn());
        }

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(res[i], f());
        }
        st.processEvent();
    }

    {
        SCOPED_TRACE("case 3. equal requests by hash after response (Input)");

        CachedEvStrategy<int, TypeList<>, TypeList<>> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            if (i == 0) {
                EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(InvokeArgument<1>());
            }
            EXPECT_CALL(res[i], f());
            st.processRequest(
                1, [rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, std::ref(valFn), res[i].fn());
            if (i == 0) {
                st.processEvent();
            }
        }
    }
}

TEST(CallStrategyTests, FullySyncEvStrategy)
{
    using Response = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    using Request = std::function<void(int, ValidationFn)>;
    const uint8_t N = 5;
    ValidationFn valFn = []() { return true; };

    {
        SCOPED_TRACE("case 1. requests before response (Input)");

        FullySyncEvStrategy<TypeList<>, TypeList<>> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            if (i == 0) {
                EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(InvokeArgument<1>());
            }
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, std::ref(valFn), res[i].fn());
        }

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(res[i], f());
            if (i < N - 1) {
                EXPECT_CALL(req[i + 1], f(i + 2, _)).WillOnce(InvokeArgument<1>());
            }
            st.processEvent();
        }
    }

    {
        SCOPED_TRACE("case 2. requests after response (Input)");

        FullySyncEvStrategy<TypeList<>, TypeList<>> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(InvokeArgument<1>());
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, std::ref(valFn), res[i].fn());
            EXPECT_CALL(res[i], f());
            st.processEvent();
        }
    }
}

TEST(CallStrategyTests, SuppressedEvStrategy)
{
    using Response = std::function<void()>;
    using ValidationFn = std::function<bool()>;
    using Request = std::function<void(int, ValidationFn)>;
    const uint8_t N = 5;
    ValidationFn valFn = []() { return true; };

    {
        SCOPED_TRACE("case 1. requests before response (Input)");

        SuppressedEvStrategy<TypeList<>, TypeList<>> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            if (i == 0) {
                EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(InvokeArgument<1>());
            }
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, std::ref(valFn), res[i].fn());
        }

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(res[i], f());
        }
        st.processEvent();
    }

    {
        SCOPED_TRACE("case 2. requests after response (Input)");

        SuppressedEvStrategy<TypeList<>, TypeList<>> st;

        StrictMock<MockedFn<Request>> req[N];
        StrictMock<MockedFn<Response>> res[N];

        ::InSequence dummy;

        for (uint8_t i = 0; i < N; ++i) {
            EXPECT_CALL(req[i], f(i + 1, _)).WillOnce(InvokeArgument<1>());
            EXPECT_CALL(res[i], f());
            st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, std::ref(valFn), res[i].fn());
            st.processEvent();
        }
    }
}

TEST(CallStrategyTests, interrupt)
{
    using Response = std::function<void()>;
    using Request = std::function<void(int, Response)>;
    const uint8_t N = 5;

    FullySyncCbStrategy<> st;

    StrictMock<MockedFn<Request>> req[N];
    StrictMock<MockedFn<Response>> res[N];

    ::InSequence dummy;

    for (uint8_t i = 0; i < N; ++i) {
        if (i == 0) {
            EXPECT_CALL(req[i], f(i + 1, _));
        }
        st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
    }

    for (uint8_t i = 0; i < N; ++i) {
        EXPECT_CALL(res[i], f());
    }
    st.interrupt();
}

TEST(CallStrategyTests, interruptImmediately)
{
    using Response = std::function<void()>;
    using Request = std::function<void(int, Response)>;
    const uint8_t N = 5;

    FullySyncCbStrategy<> st;

    StrictMock<MockedFn<Request>> req[N];
    StrictMock<MockedFn<Response>> res[N];

    ::InSequence dummy;

    for (uint8_t i = 0; i < N; ++i) {
        if (i == 0) {
            EXPECT_CALL(req[i], f(i + 1, _));
        }
        st.processRequest([rq = req[i].fn(), i](auto resp) { rq(i + 1, resp); }, res[i].fn());
    }
    st.interruptImmediately();
}
