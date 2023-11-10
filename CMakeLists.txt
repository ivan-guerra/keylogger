cmake_minimum_required(VERSION 3.27)

project(keylogger DESCRIPTION "a cross platform keylogger")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(KL_BIN_DIR
    "${CMAKE_SOURCE_DIR}/bin"
    CACHE STRING "${PROJECT_NAME} binary directory.")
set(KL_INCLUDE_DIR
    "${CMAKE_SOURCE_DIR}/include"
    CACHE STRING "${PROJECT_NAME} include directory.")

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1)
FetchContent_MakeAvailable(googletest)

if(UNIX)
  set(COMMON_FLAGS -std=c++2a)

  set(WARNING_FLAGS -Wall -Wextra -Werror -pedantic)

  set(DEBUG_FLAGS
      ${COMMON_FLAGS}
      ${WARNING_FLAGS}
      -O0
      -g3
      -ggdb
      -fno-omit-frame-pointer
      -fsanitize=address)

  set(RELEASE_FLAGS ${COMMON_FLAGS} -O2)

  add_link_options("$<$<CONFIG:Debug>:-fsanitize=address>")
elseif(WIN32)
  set(COMMON_FLAGS /std:c++20)

  set(WARNING_FLAGS /Wall)

  set(DEBUG_FLAGS ${COMMON_FLAGS} ${WARNING_FLAGS} /DEBUG:FULL)

  set(RELEASE_FLAGS ${COMMON_FLAGS} /O2)
endif()

add_compile_options("$<$<CONFIG:Release>:${RELEASE_FLAGS}>"
                    "$<$<CONFIG:Debug>:${DEBUG_FLAGS}>")

add_subdirectory(docs)
add_subdirectory(src)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(BUILD_TESTS "Build Unit Tests" OFF)
  if(BUILD_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
  endif()
endif()
