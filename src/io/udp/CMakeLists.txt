cmake_minimum_required(VERSION 3.27)

project(
  udp
  DESCRIPTION "a udp socket wrapper"
  LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE udp_socket.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${KL_INCLUDE_DIR})
