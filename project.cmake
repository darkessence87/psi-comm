# project name
message ("Working dir: ${CMAKE_CURRENT_SOURCE_DIR}")
get_filename_component (projectName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string (REPLACE " " "_" projectName ${projectName})
message ("Project: ${projectName}")

# default build type
if (NOT EXISTS ${CMAKE_BUILD_TYPE})
    set (CMAKE_BUILD_TYPE Debug)
endif()

# create build folder
set (BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
STRING (REGEX REPLACE "\\\\" "/" BUILD_DIR ${BUILD_DIR})
file (MAKE_DIRECTORY ${BUILD_DIR}/bin/)
message ("Build dir: ${BUILD_DIR}")

# create output folder
set (BUILD_OUT ${BUILD_DIR}/bin/${CMAKE_BUILD_TYPE})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_OUT})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_OUT})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_OUT})
file (MAKE_DIRECTORY ${BUILD_OUT})
message ("Build out: ${BUILD_OUT}")

# functions
function(psi_make_tests name src libs)
    include_directories (${3rdPARTY_DIR}/tests/include)
    set (test_libs "gmock;gmock_main;gtest;gtest_main")
    link_directories(${3rdPARTY_DIR}/tests/lib/${SUB_DIR_LIBS})

    set (fileName PSI_TEST_${name})
    add_executable (${fileName} ${PROJECT_SOURCE_DIR}/tests/EntryPoint.cpp ${src})
    target_link_libraries (${fileName} ${libs} ${test_libs})
endfunction()

function(psi_make_examples name src libs)
    set (fileName PSI_EXAMPLE_${name})
    add_executable (${fileName} ${src})
    target_link_libraries (${fileName} ${libs})
endfunction()