#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <stdint.h>
#include <vector>

namespace psi::comm::call_helper {

using RequestId = uint64_t;

template <typename ResponseType>
using FinishCb = std::function<void(std::vector<ResponseType>)>;

template <typename ResponseType>
using ResponseCb = std::function<void(const ResponseType &)>;

template <typename ResponseType>
using RequestFn = std::function<void(ResponseCb<ResponseType>)>;

template <typename ResponseType>
struct Request final {
    Request(RequestFn<ResponseType> f)
        : fn(f)
    {
    }
    RequestFn<ResponseType> fn;
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
 * @tparam ResponseType type of response. In fact, type of value sent in callbacks
 * @param requests list of requests
 * @param finishCb final callback to be called after all requests' callbacks are called
 */
template <typename ResponseType>
void runAll(const Requests<ResponseType> &requests, FinishCb<ResponseType> finishCb)
{
    if (requests.empty()) {
        finishCb(std::vector<ResponseType>());
        return;
    }

    using ResponsesMap = std::map<RequestId, ResponseType>;

    auto requestsN = std::make_shared<uint64_t>(requests.size());
    auto responses = std::make_shared<ResponsesMap>();

    auto onResult = [requestsN, responses, finishCb](RequestId requestId, const ResponseType &response) {
        auto itr = responses->find(requestId);
        if (itr != responses->end()) {
            itr->second = response;
        }

        --*requestsN;
        if (*requestsN == 0) {
            std::vector<ResponseType> result;
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
        req->fn([requestId, onResult](const ResponseType &response) { onResult(requestId, response); });
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
template <typename Strategy, typename ResponseType>
void runAllAsync(Strategy &strat, const Requests<ResponseType> &requests, FinishCb<ResponseType> finishCb)
{
    if (requests.empty()) {
        finishCb(std::vector<ResponseType>());
        return;
    }

    using ResponsesMap = std::map<RequestId, ResponseType>;

    auto requestsN = std::make_shared<uint64_t>(requests.size());
    auto responses = std::make_shared<ResponsesMap>();
    auto mtx = std::make_shared<std::recursive_mutex>();

    auto onResult = [requestsN, responses, finishCb, mtx](RequestId requestId, const ResponseType &response) {
        auto itr = responses->find(requestId);
        if (itr != responses->end()) {
            itr->second = response;
        }

        std::lock_guard<std::recursive_mutex> lock(*mtx);
        --*requestsN;
        if (*requestsN == 0) {
            std::vector<ResponseType> result;
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
            [=]() { req->fn([requestId, onResult](const ResponseType &response) { onResult(requestId, response); }); });
    }
}

} // namespace psi::comm::call_helper