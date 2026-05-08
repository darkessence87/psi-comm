# Description

A header-only C++ library for decoupled, reactive communication between components.

### [Subscription](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/Subscription.h)

`std::shared_ptr<Subscribable>` — a RAII handle returned by every `subscribe` call. The subscription is active as long as the handle is kept alive; destroying it automatically unsubscribes the listener.

### [Event](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/Event.h) / [IEvent](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/IEvent.h)

`Event<Args...>` — a multicast signal. Any number of listeners can subscribe; `notify(args...)` calls them all. It is safe to unsubscribe from inside a listener callback. Pass `IEvent<Args...>&` to consumers that should only listen, not fire.

```cpp
Event<int> valueChanged;
auto sub = valueChanged.subscribe([](int v){ /* ... */ });
valueChanged.notify(42);   // calls all live subscribers
sub.reset();               // unsubscribes
```

### [Attribute](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/Attribute.h) / [IAttribute](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/IAttribute.h)

`Attribute<T>` — an observable value. Fires `Event<T /*old*/, T /*new*/>` only when the value actually changes. `subscribeAndGet` immediately calls the listener with the current value, then subscribes.

```cpp
Attribute<int> counter(0);
auto sub = counter.subscribeAndGet([](int old, int now){ /* ... */ });
counter.setValue(1);  // listener called with (0, 1)
counter.setValue(1);  // no notification — value unchanged
```

Pass `IAttribute<T>&` to read-only consumers.

### [EventAsync](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/EventAsync.h) / [AttributeAsync](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/AttributeAsync.h)

Async variants that dispatch notifications through a user-provided *strategy* object (e.g. a `ThreadPool` or any object with `asyncCall(std::function<void()>)`). The caller thread fires `notify`; listeners are invoked on the strategy's thread.

```cpp
ThreadPool pool(2);
AttributeAsync<int> counter(pool);
```

### [SafeCaller](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/SafeCaller.h)

Prevents use-after-free crashes when a callback outlives its owner. Wrap member callbacks with `invoke()`; if the owner is destroyed before the callback fires, it is silently discarded (or an optional fallback is called).

```cpp
struct Client {
    Client() : m_guard(this) {}
    ~Client() { /* m_guard goes out of scope → all pending callbacks cancelled */ }

    void request(Server &s) {
        s.asyncWork(m_guard.invoke(
            [this]{ onResult(); },           // called only if Client is alive
            []{ /* fallback */ },
            "RequestName"));
    }
    SafeCaller m_guard;
};
```

### [Synched](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/Synched.h)

`Synched<T>` — mutex-guarded smart pointer. Access to the wrapped object is serialised via `operator->()`, which acquires a `std::recursive_mutex` (or a custom mutex type) for the duration of the expression.

```cpp
auto obj = std::make_shared<MyService>();
Synched<MyService> safe(obj);
safe->doSomething();  // locked access
```

### [CallHelper](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/CallHelper.h)

`call_helper::runAll(requests, finishCb)` — fires a list of async requests and calls `finishCb(results)` exactly once, after every individual callback has arrived. Order of results matches order of requests.

```cpp
using namespace psi::comm::call_helper;
Requests<bool> reqs;
reqs.emplace_back(std::make_shared<Request<bool>>([](ResponseCb<bool> cb){ cb(true); }));
runAll<bool>(reqs, [](std::vector<bool> results){ /* all done */ });
```

### [CallStrategy](https://github.com/darkessence87/psi-comm/tree/master/psi/include/psi/comm/call_strategy)

Policies for sequencing async request/response pairs. All strategies use `processRequest(requestFn, responseFn)`.

**Callback strategies** (`CbStrategy` — response is a plain callback):

| Alias | Type | Behaviour |
|---|---|---|
| `AsyncCbStrategy<CbArgs...>` | `Async` | Fire request immediately; call response immediately on reply. No ordering. |
| `FullySyncCbStrategy<CbArgs...>` | `FullySync` | Queue requests; send the next only after the previous response arrives. |
| `PartlySuppressedCbStrategy<CbArgs...>` | `PartlySuppressedSync` | Send request #1; queue the rest. On response: reply to #1..N-1 immediately, then send request #N. |
| `SuppressedCbStrategy<CbArgs...>` | `SuppressedSync` | Send request #1; queue the rest. On response: reply to #1..N-1 immediately, discard #N. |
| `CachedCbStrategy<Input, CbArgs...>` | `CachedAsync` | Deduplicate by input key via `AsyncCbStrategy`; cache result and fan out to all waiting callers. |

**Event strategies** (`EvStrategy` — response is a bool, plus an event notification):

| Alias | Type | Behaviour |
|---|---|---|
| `FullySyncEvStrategy<EvArgs, CbArgs>` | `FullySync` | Queue requests; advance only after event for the previous one. |
| `SuppressedEvStrategy<EvArgs, CbArgs>` | `SuppressedSync` | Send request #1; fan out event to all queued callers on arrival. |
| `CachedEvStrategy<Input, EvArgs, CbArgs>` | `CachedSync` | Deduplicate by input key; equal inputs share one `FullySync` slot. |

Strategies support `interrupt()` / `interruptImmediately()` for graceful shutdown.  
`Comparable` base class provides the `hashCode()`-based ordering used by cached strategies.

# Docs
[Diagrams](https://github.com/darkessence87/psi-comm/tree/master/psi/docs) created by [UMLet tool](https://www.umlet.com/)

# Usage examples
* [1_Simple_Attribute](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/1_Simple_Attribute/EntryPoint.cpp)
* [1.1_AttributeAsync](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/1.1_AttributeAsync/EntryPoint.cpp)
* [2_Simple_Event](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/2_Simple_Event/EntryPoint.cpp)
* [2.1_EventAsync](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/2.1_EventAsync/EntryPoint.cpp)
* [3_CallHelper](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/3_CallHelper/EntryPoint.cpp)
* [4_SafeCaller](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/4_SafeCaller/EntryPoint.cpp)
* [5_CallStrategy](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/5_CallStrategy/EntryPoint.cpp)
