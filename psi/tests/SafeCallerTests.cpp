
#include <gtest/gtest.h>

#include <map>
#include <queue>

#include "psi/comm/SafeCaller.h"

using namespace psi::comm;

struct Server {
    using Func = std::function<void()>;

    void request(Func &&callback)
    {
        m_callbacks.emplace(callback);
    }

    void processRequests()
    {
        while (!m_callbacks.empty()) {
            auto callback = m_callbacks.front();
            m_callbacks.pop();
            callback();
        }
    }

    std::queue<Func> m_callbacks;
};

struct Client final {
    using ClientCallback = std::function<void()>;

    Client(Server &server)
        : m_server(server)
        , m_callerGuard(reinterpret_cast<size_t>(this))
    {
    }

    void foo(ClientCallback cb)
    {
        auto fallback = []() { std::cout << "failed callback" << std::endl; };

        m_server.request(m_callerGuard.invoke(
            [cb]() {
                ++m_calledTimes;
                cb();
            },
            fallback,
            "TestRequest"));
    }

    void goo(ClientCallback cb)
    {
        m_server.request(m_callerGuard.invoke([cb]() {
            ++m_calledTimes;
            cb();
        }));
    }

    static int fooCallbackCalled()
    {
        return m_calledTimes;
    }

private:
    static int m_calledTimes;
    Server &m_server;

    SafeCaller m_callerGuard;
};

int Client::m_calledTimes = 0;

struct ClientHolder {
    int createClient(Server &server)
    {
        m_clients[++m_clientIdCounter] = std::make_shared<Client>(server);
        return m_clientIdCounter;
    }

    std::shared_ptr<Client> findClient(int clientId)
    {
        if (auto itr = m_clients.find(clientId); itr != m_clients.end()) {
            return itr->second;
        }

        return nullptr;
    }

    void deleteClient(int clientId)
    {
        if (auto itr = m_clients.find(clientId); itr != m_clients.end()) {
            m_clients.erase(itr);
        }
    }

private:
    int m_clientIdCounter = 0;
    std::map<int, std::shared_ptr<Client>> m_clients;
};

TEST(SafeCaller, TestRawPointer)
{
    Server s;

    Client *heapClient = new Client(s);
    heapClient->foo([]() {});
    heapClient->foo([]() {});
    heapClient->goo([]() {});
    delete heapClient;
    heapClient = nullptr;

    s.processRequests();

    EXPECT_EQ(Client::fooCallbackCalled(), 0);
}

TEST(SafeCaller, TestSharedPointer)
{
    Server s;

    {
        std::shared_ptr<Client> heapClient = std::make_shared<Client>(s);
        heapClient->foo([]() {});
        heapClient->foo([]() {});
        heapClient->goo([]() {});
    }

    s.processRequests();

    EXPECT_EQ(Client::fooCallbackCalled(), 0);
}

TEST(SafeCaller, TestSharedPointerArrays)
{
    ClientHolder clients;
    Server s;

    const int clientId1 = clients.createClient(s);
    {
        if (auto heapClient = clients.findClient(clientId1)) {
            heapClient->foo([]() {});
            heapClient->foo([]() {});
            heapClient->foo(std::bind(&ClientHolder::deleteClient, std::ref(clients), clientId1));
            heapClient->foo([]() {});
            heapClient->foo([]() {});
            heapClient->goo([]() {});
        }
    }

    s.processRequests();

    EXPECT_EQ(Client::fooCallbackCalled(), 3);
}
