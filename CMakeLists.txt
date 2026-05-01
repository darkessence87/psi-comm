cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED PSI_CMAKE_DIR)
    # Prefer the sibling psi-cmake checkout (sub-modules/psi-cmake) so that
    # all modules share a single source of truth. Fall back to the vendored
    # copy in 3rdparty/ only when no sibling is available (standalone clone).
    get_filename_component(_psi_sibling_cmake "${CMAKE_CURRENT_LIST_DIR}/../psi-cmake" ABSOLUTE)
    if(EXISTS "${_psi_sibling_cmake}/init.cmake")
        include(${_psi_sibling_cmake}/init.cmake)
    else()
        include(${CMAKE_CURRENT_LIST_DIR}/3rdparty/psi-cmake/init.cmake OPTIONAL RESULT_VARIABLE psi_cmake_exists)
        if(NOT psi_cmake_exists)
            message(FATAL_ERROR "psi-cmake not found")
        endif()
    endif()
endif()

include(${PSI_CMAKE_DIR}/project.cmake)
project(${projectName} LANGUAGES C CXX)
include(${PSI_CMAKE_DIR}/main.cmake)
