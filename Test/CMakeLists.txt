enable_testing()

file(GLOB TEST_SRC ./*.cpp)
file(GLOB TEST_HEADER ./*.h)
SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")

add_executable(test_runner ${TEST_SRC} ${TEST_HEADER})
target_link_libraries(test_runner gtest gtest_main pthread)