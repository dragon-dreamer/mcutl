cmake_minimum_required(VERSION 3.10)

project(mcutl LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++")
endif()

add_subdirectory(tests)