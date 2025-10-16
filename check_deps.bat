@echo off
echo --- Dependency Checker ---
echo Checking for required tools...

setlocal

:: Function to check if a command exists
where /q %1
if %errorlevel% equ 0 (
    echo %1... OK
) else (
    echo %1... NOT FOUND
    if "%1"=="git" echo   Please install Git. Visit https://git-scm.com/downloads
    if "%1"=="cmake" echo   Please install CMake. Visit https://cmake.org/download/
    if "%1"=="cl" echo   MSVC C++ compiler not found. Please install Visual Studio with C++ workload.
)

:: 1. Check for Git
call :check_command git

:: 2. Check for CMake
call :check_command cmake

:: 3. Check for a C++ compiler (MSVC)
where /q cl
if %errorlevel% equ 0 (
    echo cl... OK
) else (
    echo cl... NOT FOUND
    echo   MSVC C++ compiler not found. Please install Visual Studio with the "Desktop development with C++" workload.
)

echo.
echo --- Package Manager Checks ---
echo This project uses vcpkg for dependencies.

:: 4. Check for vcpkg
if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\\vcpkg.exe" (
        echo vcpkg... OK (Found in VCPKG_ROOT)
    ) else (
        echo vcpkg... NOT FOUND in VCPKG_ROOT
        echo   Please install vcpkg from https://vcpkg.io and set the VCPKG_ROOT environment variable.
    )
) else (
    echo vcpkg... NOT FOUND (VCPKG_ROOT is not set)
    echo   Please install vcpkg from https://vcpkg.io and set the VCPKG_ROOT environment variable.
)

echo.
echo --- Check Complete ---
echo Please review the messages above and install any missing tools.
goto :eof

:check_command
where /q %1 >nul 2>nul
if %errorlevel% equ 0 (
    echo %1... OK
) else (
    echo %1... NOT FOUND
)
goto :eof

