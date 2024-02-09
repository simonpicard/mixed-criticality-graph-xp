cmake_minimum_required(VERSION 3.22)
project(mcsexplorer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER "gcc-13")
set(CMAKE_CXX_COMPILER "g++-13")

include(CTest)
enable_testing()

set(
    WFLAGS
        "-Wall"
        "-Wextra"
        "-pedantic"
        "-Werror"
)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_compile_options("${WFLAGS}")

set(
    CPP_SOURCES
        src/State.cpp
        src/Job.cpp
        src/Graph.cpp
        src/Scheduler.cpp
        src/SafeOracle.cpp
        src/UnsafeOracle.cpp
        src/simulators/EDFSimulator.cpp
        src/oracles/safe/edf_carryoverjobs.cpp
)

add_executable(evaluation_mcs ${CPP_SOURCES} evaluation.cpp)
add_executable(edf_safe ${CPP_SOURCES} edf_safe.cpp)
add_executable(test_simulators ${CPP_SOURCES} tests/test_simulators.cpp)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
