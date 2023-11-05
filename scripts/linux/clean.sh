#!/bin/bash

source config.sh

# Remove the binary directory.
if [ -d $KL_BIN_DIR ]
then
    echo "removing '$KL_BIN_DIR'"
    rm -r $KL_BIN_DIR
fi

# Remove the CMake build directory.
if [ -d $KL_BUILD_DIR ]
then
    echo "removing '$KL_BUILD_DIR'"
    rm -rf $KL_BUILD_DIR
fi
