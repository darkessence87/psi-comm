#include "psi/comm/EventAsync.h"

#include "ExampleStrategy.h"

int main()
{
    using namespace psi;
    
    examples::ExampleStrategy strategy;

    comm::EventAsync<examples::ExampleStrategy, comm::Event<int>> eventA(strategy);

    std::mutex mtx;
    auto subscription1 = eventA.subscribe([&](const auto &value) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "1: [" << std::this_thread::get_id() << "] eventA sent " << value << std::endl;
    });
    auto subscription2 = eventA.subscribe([&](const auto &value) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "2: [" << std::this_thread::get_id() << "] eventA sent " << value << std::endl;
    });
    auto subscription3 = eventA.subscribe([&](const auto &value) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "3: [" << std::this_thread::get_id() << "] eventA sent " << value << std::endl;
    });
    auto subscription4 = eventA.subscribe([&](const auto &value) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "4: [" << std::this_thread::get_id() << "] eventA sent " << value << std::endl;
    });

    std::cout << "current thread: " << std::this_thread::get_id() << std::endl;
    // the order of subscribers is kept, but the order of notification is not guaranteed if number of asynch threads is > 1
    eventA.notify(15);
    eventA.notify(150);
    eventA.notify(1500);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}