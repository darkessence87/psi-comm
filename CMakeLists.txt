cmake_minimum_required (VERSION 3.16)

include (project.cmake)
project(${projectName} LANGUAGES C CXX)
set (3rdPARTY_DIR "${PROJECT_SOURCE_DIR}/3rdparty")

set (ENABLE_ASAN_UBSAN TRUE)

include (compiler.cmake)
include (dependencies.cmake)

add_subdirectory (psi)
