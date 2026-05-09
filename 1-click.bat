@echo off
setlocal
title Grow A Garden - 1 Click

cd /d "%~dp0"

set "BUILD_DIR=build"
set "EXE="
set "CMAKE_COMMON=-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGROW_A_GARDEN_BUILD_TESTS=OFF"

where cmake >nul 2>nul
if errorlevel 1 goto :missing_cmake

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%" || goto :build_failed
)

if exist "%BUILD_DIR%\CMakeCache.txt" (
    cmake -S . -B "%BUILD_DIR%" %CMAKE_COMMON% || goto :build_failed
) else (
    call :configure || goto :build_failed
)

cmake --build "%BUILD_DIR%" --target GrowAGarden --config Release || goto :build_failed

if exist "%BUILD_DIR%\compile_commands.json" (
    copy /Y "%BUILD_DIR%\compile_commands.json" "compile_commands.json" >nul
)

call :find_exe
if not defined EXE goto :missing_exe

start "" /D "%CD%" "%EXE%"
exit /b 0

:configure
where mingw32-make >nul 2>nul
if not errorlevel 1 (
    cmake -S . -B "%BUILD_DIR%" -G "MinGW Makefiles" %CMAKE_COMMON%
) else (
    cmake -S . -B "%BUILD_DIR%" %CMAKE_COMMON%
)
exit /b %errorlevel%

:find_exe
if exist "%BUILD_DIR%\GrowAGarden.exe" (
    set "EXE=%BUILD_DIR%\GrowAGarden.exe"
    exit /b 0
)
if exist "%BUILD_DIR%\Release\GrowAGarden.exe" (
    set "EXE=%BUILD_DIR%\Release\GrowAGarden.exe"
    exit /b 0
)
if exist "%BUILD_DIR%\Debug\GrowAGarden.exe" (
    set "EXE=%BUILD_DIR%\Debug\GrowAGarden.exe"
    exit /b 0
)
exit /b 0

:missing_cmake
echo.
echo CMake was not found on PATH.
echo Install CMake, then double-click this file again.
pause
exit /b 1

:missing_exe
echo.
echo Build finished, but GrowAGarden.exe was not found in the build folder.
pause
exit /b 1

:build_failed
echo.
echo Grow A Garden could not be built or launched.
echo Check the messages above, then double-click this file again after fixing them.
pause
exit /b 1
