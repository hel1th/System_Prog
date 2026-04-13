cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(sys_prog LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message([[VCPKG toolchain setup example. It is needed for some libraries. Highly recommended to pass it by command line arguments for cmake configuration (IDE can help with this).
        set(VCPKG_TARGET_TRIPLET x64-windows)
        set(CMAKE_TOOLCHAIN_FILE C:/Programming/vcpkg/scripts/buildsystems/vcpkg.cmake)]])

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
FetchContent_MakeAvailable(googletest)

find_package(Boost REQUIRED COMPONENTS system container)

if(Boost_FOUND)
    message(STATUS "✅ Boost found: ${Boost_INCLUDE_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "❌ Boost not found! Check triplet and installation.")
endif()

add_subdirectory(allocator)
add_subdirectory(associative_container)
add_subdirectory(common)