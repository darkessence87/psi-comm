#include "psi/comm/SafeCaller.h"

#include <functional>

int main()
{
    using namespace psi;

    class Server final
    {
    public:
        using BoolCallback = std::function<void(bool)>;

        ~Server()
        {
            std::cout << "[server] destroyed" << std::endl;
        }

        void connect(BoolCallback &&cb)
        {
            std::cout << "[server] connecting client..." << std::endl;
            m_clientConnectionCb = cb;
        }

        void finishClientConnection()
        {
            if (m_clientConnectionCb) {
                std::cout << "[server] connected client" << std::endl;
                m_clientConnectionCb(true);
            }
        }

        BoolCallback m_clientConnectionCb = nullptr;
    };

    class Client final
    {
    public:
        using BoolCallback = std::function<void(bool)>;

        Client(Server *server)
            : m_guard(this)
            , m_server(server)
        {
        }

        ~Client()
        {
            std::cout << "[client] destroyed" << std::endl;
        }

        void connect()
        {
            std::cout << "[client] connecting..." << std::endl;
            m_server->connect(m_guard.invoke([this](bool result) { onConnected(result); }));
        }

        void connect_with_fallback()
        {
            std::cout << "[client] connecting with fallback..." << std::endl;
            m_server->connect(m_guard.invoke([this](bool result) { onConnected(result); },
                                             []() { std::cout << "fallback()" << std::endl; },
                                             "onConnected"));
        }

        void onConnected(bool isConnected)
        {
            std::cout << "[client] connected: " << std::boolalpha << isConnected << std::endl;
        }

        comm::SafeCaller m_guard;
        Server *m_server = nullptr;
    };

    /// basic SafeCaller usage case
    {
        Server *server = new Server();
        Client *client = new Client(server);

        // good case
        std::cout << "good case" << std::endl;
        client->connect();
        server->finishClientConnection();

        // bad case, what if client is removed before server answers callback
        // safe caller protects the invoker of crash, as lambda captured 'this' in method 'connect'
        std::cout << "bad case" << std::endl;
        client->connect();
        delete client;
        server->finishClientConnection();
        delete server;
    }

    /// SafeCaller usage case with fallback. Fallback might be used when we need to call externally provided call.
    /// Fallback must not capture 'this', as it is called instead of main callback
    {
        Server *server = new Server();
        Client *client = new Client(server);
        std::cout << "bad case" << std::endl;
        client->connect_with_fallback();
        delete client;
        server->finishClientConnection();
        delete server;
    }
}