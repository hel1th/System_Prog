cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

# Fix for Boost warning
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(sys_prog LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost CONFIG REQUIRED COMPONENTS system container)
message(STATUS "✅ Boost ${Boost_VERSION} found")

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(allocator)
add_subdirectory(associative_container)
add_subdirectory(common)