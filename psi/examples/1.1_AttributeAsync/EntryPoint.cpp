#include "psi/comm/AttributeAsync.h"

#include "ExampleStrategy.h"

int main()
{
    using namespace psi;

    examples::ExampleStrategy strategy;

    std::mutex mtx;
    comm::AttributeAsync<int> valueA(strategy);
    auto subscription1 = valueA.subscribe([&](const auto &oldValue, const auto &newValue) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "1: [" << std::this_thread::get_id() << "] valueA changed " << oldValue << "->" << newValue << std::endl;
    });
    auto subscription2 = valueA.subscribe([&](const auto &oldValue, const auto &newValue) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "2: [" << std::this_thread::get_id() << "] valueA changed " << oldValue << "->" << newValue << std::endl;
    });
    auto subscription3 = valueA.subscribe([&](const auto &oldValue, const auto &newValue) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "3: [" << std::this_thread::get_id() << "] valueA changed " << oldValue << "->" << newValue << std::endl;
    });
    auto subscription4 = valueA.subscribe([&](const auto &oldValue, const auto &newValue) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "4: [" << std::this_thread::get_id() << "] valueA changed " << oldValue << "->" << newValue << std::endl;
    });

    std::cout << "current thread: " << std::this_thread::get_id() << std::endl;
    // the order of subscribers is kept, but the order of notification is not guaranteed if number of asynch threads is > 1
    valueA.setValue(15);
    valueA.setValue(150);
    valueA.setValue(15);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}