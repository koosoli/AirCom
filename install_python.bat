@echo off
echo ========================================
echo AirCom Python Installation Script
echo ========================================
echo This script installs Python manually for AirCom development
echo.

echo Checking if Python is already installed...
python --version >nul 2>&1
if %errorlevel% equ 0 (
    echo Python is already installed!
    python --version
    goto :install_pip
)

echo.
echo Python not found. Installing Python 3.11.6 manually...
echo.

REM Download Python installer
echo Downloading Python 3.11.6 installer...
powershell -Command "Invoke-WebRequest -Uri 'https://www.python.org/ftp/python/3.11.6/python-3.11.6-amd64.exe' -OutFile 'python_installer.exe'"

if not exist "python_installer.exe" (
    echo ERROR: Failed to download Python installer
    echo Please download Python manually from https://www.python.org/downloads/
    goto :error
)

echo.
echo Installing Python 3.11.6...
echo IMPORTANT: When prompted, make sure to check:
echo - "Add python.exe to PATH"
echo - "Install for all users" (optional)
echo.

REM Run the installer
python_installer.exe /quiet InstallAllUsers=1 PrependPath=1 Include_test=0

if %errorlevel% neq 0 (
    echo ERROR: Python installation failed
    echo Please run the installer manually: python_installer.exe
    goto :error
)

echo.
echo Cleaning up installer...
del python_installer.exe

echo.
echo Verifying Python installation...
python --version
if %errorlevel% neq 0 (
    echo ERROR: Python verification failed
    goto :error
)

:install_pip
echo.
echo Installing/updating pip...
python -m ensurepip --upgrade
python -m pip install --upgrade pip

:install_platformio
echo.
echo Installing PlatformIO...
python -m pip install platformio

echo.
echo Verifying PlatformIO installation...
pio --version
if %errorlevel% neq 0 (
    echo ERROR: PlatformIO installation failed
    goto :error
)

echo.
echo ========================================
echo Installation completed successfully!
echo ========================================
echo.
echo Python and PlatformIO are now installed.
echo You can now run the build scripts.
echo.
goto :eof

:error
echo.
echo ========================================
echo INSTALLATION FAILED!
echo ========================================
echo.
echo Please install Python and PlatformIO manually:
echo 1. Download Python from: https://www.python.org/downloads/
echo 2. Install PlatformIO: python -m pip install platformio
echo 3. Run this script again
echo.
pause