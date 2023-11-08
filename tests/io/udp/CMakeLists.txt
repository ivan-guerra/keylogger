cmake_minimum_required(VERSION 3.27)

set(TESTNAME udp_socket_test)

add_executable(${TESTNAME} ${TESTNAME}.cpp)

target_link_libraries(${TESTNAME} PRIVATE gtest_main udp)

gtest_discover_tests(${TESTNAME} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
