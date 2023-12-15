#include "psi/comm/Attribute.h"

int main()
{
    using namespace psi;

    comm::Attribute<int> valueA(100);
    auto subscription = valueA.subscribe([](const auto &oldValue, const auto &newValue) {
        std::cout << "valueA changed " << oldValue << "->" << newValue << std::endl;
    });

    valueA.setValue(15);
    valueA.setValue(150);
    valueA.setValue(15);
}