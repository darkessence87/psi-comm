
include_directories(
    include
)

set(TEST_SRC
    tests/AttributeTests.cpp
    tests/CallHelperTests.cpp
    tests/CallStrategyTests.cpp
    tests/EventTests.cpp
    tests/SafeCallerTests.cpp
    tests/SynchedTests.cpp
)
psi_make_tests("Comm" "${TEST_SRC}" "")

set(EXAMPLE_SRC examples/1_Simple_Attribute/EntryPoint.cpp)
psi_make_examples("1_Simple_Attribute" "${EXAMPLE_SRC}" "")

set(EXAMPLE_SRC examples/1.1_AttributeAsync/EntryPoint.cpp)
psi_make_examples("1.1_AttributeAsync" "${EXAMPLE_SRC}" "")

set(EXAMPLE_SRC examples/2_Simple_Event/EntryPoint.cpp)
psi_make_examples("2_Simple_Event" "${EXAMPLE_SRC}" "")

set(EXAMPLE_SRC examples/2.1_EventAsync/EntryPoint.cpp)
psi_make_examples("2.1_EventAsync" "${EXAMPLE_SRC}" "")

set(EXAMPLE_SRC examples/3_CallHelper/EntryPoint.cpp)
psi_make_examples("3_CallHelper" "${EXAMPLE_SRC}" "")

set(EXAMPLE_SRC examples/4_SafeCaller/EntryPoint.cpp)
psi_make_examples("4_SafeCaller" "${EXAMPLE_SRC}" "")

set(EXAMPLE_SRC examples/5_CallStrategy/EntryPoint.cpp)
psi_make_examples("5_CallStrategy" "${EXAMPLE_SRC}" "")