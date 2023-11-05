cmake_minimum_required(VERSION 3.27)

if(UNIX)
  find_package(X11 REQUIRED)
endif(UNIX)

project(
  recorder
  DESCRIPTION "hook into and record system keypress events"
  LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE event_loop.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${KL_INCLUDE_DIR})

if(UNIX)
  target_link_libraries(${PROJECT_NAME} PRIVATE X11::X11 X11::Xtst)
endif()
