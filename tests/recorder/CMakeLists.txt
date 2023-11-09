cmake_minimum_required(VERSION 3.27)

set(TESTNAME recorder_test)

add_executable(${TESTNAME} ${TESTNAME}.cpp)

target_link_libraries(${TESTNAME} PRIVATE gtest_main recorder udp)

gtest_discover_tests(${TESTNAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
