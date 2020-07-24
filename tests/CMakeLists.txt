cmake_minimum_required(VERSION 3.10)

include(GoogleTest)

file(GLOB TESTS_SOURCES "*.h" "*.cpp")

add_executable(tests ${TESTS_SOURCES}
	"${PROJECT_SOURCE_DIR}/gtest/src/gtest-all.cc"
	"${PROJECT_SOURCE_DIR}/gmock/src/gmock-all.cc")

target_include_directories(tests PUBLIC
	"${PROJECT_SOURCE_DIR}/"
	"${PROJECT_SOURCE_DIR}/gtest"
	"${PROJECT_SOURCE_DIR}/gmock")

target_compile_definitions(tests PUBLIC MCUTL_TEST)

target_compile_options(tests PUBLIC -Wall -Wextra)

set(PTHREAD_LIB -pthread)
target_link_libraries(tests ${PTHREAD_LIB})

gtest_discover_tests(tests)