@echo off
echo ========================================
echo AirCom Quick Build Script
echo ========================================
echo This script builds AirCom using existing tools
echo.

echo Step 1: Check if Python is available...
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Python not found!
    echo.
    echo Please install Python first:
    echo 1. Download from: https://www.python.org/downloads/
    echo 2. Install Python 3.11.x
    echo 3. Make sure to check "Add to PATH" during installation
    echo 4. Restart your command prompt
    echo 5. Run this script again
    goto :eof
)

echo Python found!
python --version

echo.
echo Step 2: Check if PlatformIO is available...
pio --version >nul 2>&1
if %errorlevel% neq 0 (
    echo PlatformIO not found. Installing...
    python -m pip install platformio
    if %errorlevel% neq 0 (
        echo ERROR: Failed to install PlatformIO
        echo Please install manually: python -m pip install platformio
        goto :eof
    )
)

echo PlatformIO found!
pio --version

echo.
echo Step 3: Navigate to project directory...
cd /d %~dp0
echo Current directory: %cd%

echo.
echo Step 4: Build for XIAO ESP32S3...
echo Building AirCom firmware...
pio run -e xiao_esp32s3

if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    echo Check the error messages above for details.
    goto :error
)

echo.
echo ========================================
echo BUILD SUCCESSFUL! ✅
echo ========================================
echo.
echo Firmware built successfully!
echo Location: .pio\build\xiao_esp32s3\firmware.bin
echo.
echo To upload to device, connect your ESP32 and run:
echo pio run -e xiao_esp32s3 --target upload
echo.
echo Other build targets available:
echo pio run -e xiao_esp32c3     # XIAO ESP32C3
echo pio run -e xiao_esp32c6     # XIAO ESP32C6
echo pio run -e heltec_ht_hc32   # Heltec HT-HC32
echo pio run -e heltec_ht_it01   # Heltec HT-IT01
echo pio run -e generic_heltec   # Generic Heltec
echo.
goto :eof

:error
echo.
echo ========================================
echo BUILD FAILED! ❌
echo ========================================
echo.
echo Possible solutions:
echo 1. Check error messages above
echo 2. Ensure all files are present
echo 3. Verify internet connection for library downloads
echo 4. Check that protobuf files were generated (run generate_protobuf.bat if needed)
echo.
echo For detailed troubleshooting, see BUILD_README.md
echo.