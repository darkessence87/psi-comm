cmake_minimum_required (VERSION 3.6)

include (project.cmake)
project(${projectName})
set (3rdPARTY_DIR "${PROJECT_SOURCE_DIR}/3rdparty")

include (compiler.cmake)
include (dependencies.cmake)

add_subdirectory (psi)