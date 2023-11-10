@echo off

REM Root directory.
set KL_PROJECT_PATH=%cd%\..\..

REM Binary directory.
set KL_BIN_DIR=%KL_PROJECT_PATH%\bin

REM CMake build files.
set KL_BUILD_DIR=%KL_PROJECT_PATH%\build

REM Location of external project sources.
set KL_EXTERN_DIR=%KL_PROJECT_PATH%\extern
