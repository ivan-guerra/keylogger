cmake_minimum_required(VERSION 3.27)

project(
  udp
  DESCRIPTION "a udp socket wrapper"
  LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

if(UNIX)
  target_sources(${PROJECT_NAME} PRIVATE linux_udp_socket.cpp)
elseif(WIN32)
  target_sources(${PROJECT_NAME} PRIVATE windows_udp_socket.cpp)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${KL_INCLUDE_DIR})

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()
