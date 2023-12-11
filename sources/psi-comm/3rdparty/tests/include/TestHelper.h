#pragma once

#include <gmock/gmock.h>

#include <chrono>
#include <functional>

namespace psi::test {

// class TestHelper
// {
// public:
//     static void timeFn(const auto &name, auto &&fn, int N)
//     {
//         using namespace std::chrono;

//         const auto &start = high_resolution_clock::now();
//         for (int i = 0; i < N; ++i) {
//             fn();
//         }
//         const auto &end = high_resolution_clock::now();
//         const auto &totalTime = (end - start).count();
//         std::cout << "[" << name << "] average fn() us: " << totalTime / 1000.0 / N << std::endl;
//     }
// };

template <typename C>
struct MockedFn;

template <typename C>
struct TestFn;

template <typename R, typename... Args>
struct TestFn<std::function<R(Args...)>> {
    virtual ~TestFn() = default;

    using Fn = std::function<R(Args...)>;

    static std::shared_ptr<MockedFn<std::function<R(Args...)>>> create()
    {
        return std::make_shared<MockedFn<std::function<R(Args...)>>>();
    }

    Fn fn()
    {
        return [this](Args... args) { f(std::forward<Args>(args)...); };
    }

    virtual R f(Args...) const = 0;
};

template <typename R>
struct MockedFn<std::function<R()>> : TestFn<std::function<R()>> {
    MOCK_METHOD(R, f, (), (const, override));
};

template <typename R, typename A1>
struct MockedFn<std::function<R(A1)>> : TestFn<std::function<R(A1)>> {
    MOCK_METHOD(R, f, (A1), (const, override));
};

template <typename R, typename A1, typename A2>
struct MockedFn<std::function<R(A1, A2)>> : TestFn<std::function<R(A1, A2)>> {
    MOCK_METHOD(R, f, (A1, A2), (const, override));
};

// template <typename R, typename... Args>
// struct MockedFn<std::function<R(Args...)>> : TestFn<std::function<R(Args...)>> {
//     MOCK_METHOD(R, f, (Args...), (const, override));
// };

} // namespace psi