set(target_lib "psi-comm")

# Header-only library: expose include/ via an INTERFACE target.
add_library(${target_lib} INTERFACE)
add_library(psi::comm ALIAS ${target_lib})

target_include_directories(${target_lib}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(${target_lib} INTERFACE cxx_std_20)

set(TEST_SRC
    tests/Attribute_Tests.cpp
    tests/CallHelper_Tests.cpp
    # tests/CallStrategyTests.cpp
    tests/Event_Tests.cpp
    tests/SafeCaller_Tests.cpp
    # tests/SynchedTests.cpp
)
psi_make_tests("Comm" "${TEST_SRC}" "${target_lib}")

set(EXAMPLE_SRC examples/1_Simple_Attribute/EntryPoint.cpp)
psi_make_examples("1_Simple_Attribute" "${EXAMPLE_SRC}" "${target_lib}")

set(EXAMPLE_SRC examples/1.1_AttributeAsync/EntryPoint.cpp)
psi_make_examples("1.1_AttributeAsync" "${EXAMPLE_SRC}" "${target_lib}")

set(EXAMPLE_SRC examples/2_Simple_Event/EntryPoint.cpp)
psi_make_examples("2_Simple_Event" "${EXAMPLE_SRC}" "${target_lib}")

set(EXAMPLE_SRC examples/2.1_EventAsync/EntryPoint.cpp)
psi_make_examples("2.1_EventAsync" "${EXAMPLE_SRC}" "${target_lib}")

set(EXAMPLE_SRC examples/3_CallHelper/EntryPoint.cpp)
psi_make_examples("3_CallHelper" "${EXAMPLE_SRC}" "${target_lib}")

set(EXAMPLE_SRC examples/4_SafeCaller/EntryPoint.cpp)
psi_make_examples("4_SafeCaller" "${EXAMPLE_SRC}" "${target_lib}")

set(EXAMPLE_SRC examples/5_CallStrategy/EntryPoint.cpp)
psi_make_examples("5_CallStrategy" "${EXAMPLE_SRC}" "${target_lib}")