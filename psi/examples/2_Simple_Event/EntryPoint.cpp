#include "psi/comm/Event.h"

int main()
{
    using namespace psi;

    comm::Event<int> eventA;
    auto subscription = eventA.subscribe([](const auto &value) {
        std::cout << "eventA sent: " << value << std::endl;
    });

    eventA.notify(15);
    eventA.notify(150);
    eventA.notify(15);
}