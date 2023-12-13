include_directories(
    ${3rdPARTY_DIR}/psi/include
    ${3rdPARTY_DIR}/psi-tools/psi/include
    include
)

set(TEST_SRC
    tests/AttributeTests.cpp
    tests/SafeCallerTests.cpp
    tests/SynchedTests.cpp
)
psi_make_tests("Comm" "${TEST_SRC}" "")

set(EXAMPLE_SRC
    examples/1_Simple_Attribute/EntryPoint.cpp
)
psi_make_examples("1_Simple_Attribute" "${EXAMPLE_SRC}")