@echo off

call config.bat

REM Supported build types are "RELEASE" and "DEBUG".
set BUILD_TYPE="RELEASE"
if not "%1"=="" set BUILD_TYPE=%1

if not exist %KL_BIN_DIR% mkdir %KL_BIN_DIR%

REM Generate the keylogger Visual Studio 2022 solution.
cmake ^
    -G "Visual Studio 17 2022" ^
    -DBUILD_DOCS=ON ^
    -DBUILD_TESTS=ON ^
    -DCMAKE_INSTALL_PREFIX=%KL_BIN_DIR% ^
    -B %KL_BUILD_DIR% ^
    -S %KL_PROJECT_PATH%

cmake ^
    --build %KL_BUILD_DIR% ^
    --config %BUILD_TYPE%

cmake ^
    --install %KL_BUILD_DIR% ^
    --config %BUILD_TYPE%
