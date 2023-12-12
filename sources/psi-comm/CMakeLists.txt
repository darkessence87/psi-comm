cmake_minimum_required(VERSION 3.6)

STRING(REGEX REPLACE "\\\\" "/" BUILD_DIR ${BUILD_DIR} )
message("Build dir: ${BUILD_DIR}")

# setup configuration parameters
file(MAKE_DIRECTORY ${BUILD_DIR}/bin/)

if (WIN32)
    # setup project
    project(psi-sm)

    set( SUB_DIR_LIBS ${CMAKE_BUILD_TYPE}/win64)

    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std:c++20 -W4 /MP /O2")
    # set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
    set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
else()
    message("Only Windows build is supported at the moment.")
    return()
endif()

message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

set( BUILD_OUT ${BUILD_DIR}/bin/${CMAKE_BUILD_TYPE})
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BUILD_OUT})
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BUILD_OUT})
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_OUT})
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_OUT})
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${BUILD_OUT})
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${BUILD_OUT})

# cleanups
file(MAKE_DIRECTORY ${BUILD_OUT})
message("Build out: ${BUILD_OUT}")

# 3rd parties
set( 3rdPARTY_DIR "${PROJECT_SOURCE_DIR}/3rdparty")

if (EXISTS ${3rdPARTY_DIR}/psi/include/psi/logger)
    message("psi-logger found")
    add_compile_definitions(PSI_LOGGER)
    link_directories(${3rdPARTY_DIR}/psi/lib/${SUB_DIR_LIBS})
    link_libraries(psi-logger psi-shared)
endif()

# functions
function(psi_make_tests name src libs)
    if (WIN32)
        set (PLATFORM_LIBS "dbghelp.dll")
    endif()

    set( fileName PSI_TEST_${name})
    include_directories(
        ${3rdPARTY_DIR}/psi/include
        ${3rdPARTY_DIR}/tests/include
    )
    link_directories(
        ${3rdPARTY_DIR}/tests/lib/${SUB_DIR_LIBS}
    )
    add_executable(${fileName} ${PROJECT_SOURCE_DIR}/tests/EntryPoint.cpp ${src})
    target_link_libraries(${fileName}
        ${libs}
        ${PLATFORM_LIBS}
        gmock
        gmock_main
        gtest
        gtest_main
    )
endfunction()

function(psi_make_examples name src)
    set( fileName PSI_EXAMPLE_${name})
    add_executable(${fileName} ${src})
endfunction()

add_subdirectory(psi)