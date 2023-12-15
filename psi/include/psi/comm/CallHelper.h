#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <stdint.h>
#include <vector>

namespace psi::comm::call_helper {

using RequestId = uint64_t;

template <typename T>
using FinishCb = std::function<void(std::vector<T>)>;

template <typename T>
using ResponseCb = std::function<void(const T &)>;

template <typename T>
using RequestFn = std::function<void(ResponseCb<T>)>;

template <typename Response>
struct Request final {
    Request(RequestFn<Response> f)
        : fn(f)
    {
    }
    RequestFn<Response> fn;
};

template <typename T>
using RequestPtr = std::shared_ptr<Request<T>>;

template <typename T>
using Requests = std::vector<RequestPtr<T>>;

/**
 * @brief This function processes provided list of requests in a sequence.
 * Requests are functions with own callback.
 * Final callback will be called only after callbacks for all requestes are called.
 * 
 * @tparam Response type of response. In fact, type of value sent in callbacks
 * @param requests list of requests
 * @param finishCb final callback to be called after all requests' callbacks are called
 */
template <typename Response>
void runAll(const Requests<Response> &requests, FinishCb<Response> finishCb)
{
    if (requests.empty()) {
        finishCb(std::vector<Response>());
        return;
    }

    using ResponsesMap = std::map<RequestId, Response>;

    auto requestsN = std::make_shared<uint64_t>(requests.size());
    auto responses = std::make_shared<ResponsesMap>();

    auto onResult = [requestsN, responses, finishCb](RequestId requestId, const Response &response) {
        auto itr = responses->find(requestId);
        if (itr != responses->end()) {
            itr->second = response;
        }

        --*requestsN;
        if (*requestsN == 0) {
            std::vector<Response> result;
            for (const auto &res : *responses.get()) {
                result.emplace_back(res.second);
            }

            finishCb(result);
        }
    };

    int requestId = 0;
    for (const auto &req : requests) {
        ++requestId;
        (*responses)[requestId];
        req->fn([requestId, onResult](const Response &response) { onResult(requestId, response); });
    }
}

/**
 * @brief This function processes provided list of requests asynchronously.
 * Requests are functions with own callback.
 * Final callback will be called only after callbacks for all requestes are called.
 * 
 * @tparam Strategy type of class implements asyncronous calls
 * @tparam Response type of response. In fact, type of value sent in callbacks
 * @param strat object of Strategy class which invokes provided functions asynchronously
 * @param requests list of requests
 * @param finishCb final callback to be called after all requests' callbacks are called
 */
template <typename Strategy, typename Response>
void runAllAsync(Strategy &strat, const Requests<Response> &requests, FinishCb<Response> finishCb)
{
    if (requests.empty()) {
        finishCb(std::vector<Response>());
        return;
    }

    using ResponsesMap = std::map<RequestId, Response>;

    auto requestsN = std::make_shared<uint64_t>(requests.size());
    auto responses = std::make_shared<ResponsesMap>();
    auto mtx = std::make_shared<std::recursive_mutex>();

    auto onResult = [requestsN, responses, finishCb, mtx](RequestId requestId, const Response &response) {
        auto itr = responses->find(requestId);
        if (itr != responses->end()) {
            itr->second = response;
        }

        std::lock_guard<std::recursive_mutex> lock(*mtx);
        --*requestsN;
        if (*requestsN == 0) {
            std::vector<Response> result;
            for (const auto &res : *responses.get()) {
                result.emplace_back(res.second);
            }

            finishCb(result);
        }
    };

    int requestId = 0;
    for (const auto &req : requests) {
        ++requestId;
        (*responses)[requestId];
        strat.asyncCall(
            [=]() { req->fn([requestId, onResult](const Response &response) { onResult(requestId, response); }); });
    }
}

} // namespace psi::comm::call_helper