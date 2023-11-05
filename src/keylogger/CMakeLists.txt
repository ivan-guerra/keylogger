cmake_minimum_required(VERSION 3.27)

add_executable(${CMAKE_PROJECT_NAME})

target_sources(${CMAKE_PROJECT_NAME} PRIVATE keylogger.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE recorder)

install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION ${KL_BIN_DIR})
