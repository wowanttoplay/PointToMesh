@echo off
setlocal

echo --- Visual Studio Developer Environment Launcher for VS Code ---

:: Find vswhere.exe
set "VSWHERE_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE_PATH%" (
    echo [ERROR] vswhere.exe not found at "%VSWHERE_PATH%".
    echo Please ensure Visual Studio is installed correctly.
    pause
    exit /b 1
)

:: Find the latest Visual Studio installation path
for /f "usebackq tokens=*" %%i in (`"%VSWHERE_PATH%" -latest -property installationPath`) do (
    set "VS_INSTALL_PATH=%%i"
)

if not defined VS_INSTALL_PATH (
    echo [ERROR] Could not find any Visual Studio installation.
    pause
    exit /b 1
)

:: Define the path to VsDevCmd.bat
set "VS_DEV_CMD_PATH=%VS_INSTALL_PATH%\Common7\Tools\VsDevCmd.bat"
if not exist "%VS_DEV_CMD_PATH%" (
    echo [ERROR] VsDevCmd.bat not found in the detected Visual Studio installation.
    echo Looked for it at: "%VS_DEV_CMD_PATH%"
    pause
    exit /b 1
)

echo Found Visual Studio at: %VS_INSTALL_PATH%
echo Launching VS Code within the developer environment...
echo This may take a moment. A new window will open.

:: Launch a new command prompt that first initializes the VS environment, then starts VS Code.
start "VS Code Developer Environment" cmd /c ""%VS_DEV_CMD_PATH%" && code ."

endlocal

