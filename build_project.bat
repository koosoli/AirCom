@echo off
echo ========================================
echo AirCom Project Builder
echo ========================================
echo This script builds the AirCom project and resolves dependencies
echo.

REM Check if PlatformIO is installed
pio --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: PlatformIO not found!
    echo Please run build_setup.bat first to install PlatformIO
    goto :eof
)

echo Step 1: Installing PlatformIO dependencies...
cd /d %~dp0

echo Installing ESP-IDF components...
pio pkg install --tool espressif/toolchain-xtensa32
pio pkg install --tool espressif/toolchain-esp32ulp

echo Step 2: Installing project dependencies...
pio pkg install --library "madhephaestus/ESP32AnalogRead"
pio pkg install --library "me-no-dev/AsyncTCP"
pio pkg install --library "me-no-dev/ESPAsyncWebServer"

echo Step 3: Building for all supported platforms...
echo.

echo Building for XIAO ESP32S3...
pio run -e xiao_esp32s3
if errorlevel 1 (
    echo ERROR: Build failed for XIAO ESP32S3
    goto :build_error
)

echo Building for XIAO ESP32C3...
pio run -e xiao_esp32c3
if errorlevel 1 (
    echo ERROR: Build failed for XIAO ESP32C3
    goto :build_error
)

echo Building for XIAO ESP32C6...
pio run -e xiao_esp32c6
if errorlevel 1 (
    echo ERROR: Build failed for XIAO ESP32C6
    goto :build_error
)

echo Building for Heltec HT-HC32...
pio run -e heltec_ht_hc32
if errorlevel 1 (
    echo ERROR: Build failed for Heltec HT-HC32
    goto :build_error
)

echo Building for Heltec HT-IT01...
pio run -e heltec_ht_it01
if errorlevel 1 (
    echo ERROR: Build failed for Heltec HT-IT01
    goto :build_error
)

echo Building for Generic Heltec...
pio run -e generic_heltec
if errorlevel 1 (
    echo ERROR: Build failed for Generic Heltec
    goto :build_error
)

echo.
echo ========================================
echo All builds completed successfully!
echo ========================================
echo.
echo Firmware files are located in .pio/build/ directories
echo.
echo To upload firmware:
echo pio run -e xiao_esp32s3 --target upload
echo.
goto :eof

:build_error
echo.
echo ========================================
echo BUILD FAILED!
echo ========================================
echo.
echo Possible solutions:
echo 1. Check that all dependencies are installed
echo 2. Verify ESP-IDF installation
echo 3. Check for missing components
echo 4. Review error messages above
echo.
echo For detailed troubleshooting, see README.md
echo.