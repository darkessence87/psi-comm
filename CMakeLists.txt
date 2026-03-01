cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED PSI_CMAKE_DIR)
    include(${CMAKE_CURRENT_LIST_DIR}/3rdparty/psi-cmake/init.cmake OPTIONAL RESULT_VARIABLE psi_cmake_exists)
    if(NOT psi_cmake_exists)
        message(FATAL_ERROR "psi-cmake not found")
    endif()
endif()

include(${PSI_CMAKE_DIR}/project.cmake)
project(${projectName} LANGUAGES C CXX)
include(${PSI_CMAKE_DIR}/main.cmake)
