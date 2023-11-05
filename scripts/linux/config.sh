#!/bin/bash

CWD=$(pwd)

# Root directory.
KL_PROJECT_PATH=$(dirname $(dirname ${CWD}))

# Scripts directory.
KL_SCRIPTS_PATH="${KL_PROJECT_PATH}/scripts/linux"

# Binary directory.
KL_BIN_DIR="${KL_PROJECT_PATH}/bin"

# Resource directory.
KL_RES_DIR="${KL_PROJECT_PATH}/resources"

# CMake build files and cache.
KL_BUILD_DIR="${KL_PROJECT_PATH}/build"
