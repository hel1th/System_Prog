cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(sys_prog LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message([[VCPKG toolchain setup example. It is needed for some libraries. Highly recommended to pass it by command line arguments for cmake configuration (IDE can help with this).
        set(VCPKG_TARGET_TRIPLET x64-windows)
        set(CMAKE_TOOLCHAIN_FILE C:/Programming/vcpkg/scripts/buildsystems/vcpkg.cmake)]])

find_package(GTest REQUIRED)

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS container REQUIRED) # Ставить через vcpkg

add_subdirectory(allocator)
add_subdirectory(associative_container)
add_subdirectory(common)