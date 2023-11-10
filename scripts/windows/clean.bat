@echo off

call config.bat

if exist %KL_BIN_DIR% rmdir /s /q %KL_BIN_DIR% & @echo "removed %KL_BIN_DIR%"
if exist %KL_BUILD_DIR% rmdir /s /q %KL_BUILD_DIR% & @echo "removed %KL_BUILD_DIR%"
