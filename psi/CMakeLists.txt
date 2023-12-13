if (NOT EXISTS ${psi_tools_dir})
    set (psi_tools_dir "")
    find_submodule(psi-tools psi_tools_dir)
    message("psi_tools_dir: ${psi_tools_dir}")
endif()

include_directories(
    ${3rdPARTY_DIR}/psi/include
    ${psi_tools_dir}/psi/include
    include
)

link_directories(
    ${3rdPARTY_DIR}/psi/lib/${SUB_DIR_LIBS}
    ${psi_tools_dir}/build/bin/${CMAKE_BUILD_TYPE}
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
psi_make_examples("1_Simple_Attribute" "${EXAMPLE_SRC}" "")