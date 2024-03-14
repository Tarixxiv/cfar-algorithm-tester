cmake_minimum_required(VERSION 3.27)
project(cfar-algorithm-tester)

set(CMAKE_CXX_STANDARD 14)
#
#add_executable(cfaragain main.cpp)
enable_testing()

# Include Google Test using FetchContent
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)


# Add your source files
add_executable(my_test mytest.cpp)
#add_executable(my_test CFAR.cpp)

# Link with Google Test and any other dependencies
target_link_libraries(my_test gtest gtest_main)

add_test(random_string ./my_test)