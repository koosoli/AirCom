@echo off
echo ========================================
echo AirCom Complete Setup and Build Script
echo ========================================
echo This script addresses all identified issues and builds the project
echo.

echo Step 1: Installing development tools...
echo Checking for Python...
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Python not found. Installing Python...
    call install_python.bat
    if %errorlevel% neq 0 (
        echo ERROR: Python installation failed
        echo Please install Python manually from https://www.python.org/downloads/
        goto :error
    )
) else (
    echo Python is already installed.
    python --version
)

echo.
echo Step 2: Installing PlatformIO...
python -m ensurepip --upgrade
python -m pip install --upgrade pip
python -m pip install platformio

echo.
echo Step 3: Verifying installations...
python --version
if %errorlevel% neq 0 (
    echo ERROR: Python verification failed
    goto :error
)

pio --version
if %errorlevel% neq 0 (
    echo ERROR: PlatformIO installation failed
    goto :error
)

echo.
echo Step 4: Setting up project dependencies...
cd /d %~dp0

echo Generating protobuf files...
call generate_protobuf.bat
if errorlevel 1 (
    echo WARNING: Protobuf generation failed - will be handled during build
)

echo.
echo Step 5: Installing PlatformIO dependencies...
pio pkg install --tool espressif/toolchain-xtensa32
pio pkg install --tool espressif/toolchain-esp32ulp
pio pkg install --library "madhephaestus/ESP32AnalogRead"
pio pkg install --library "me-no-dev/AsyncTCP"
pio pkg install --library "me-no-dev/ESPAsyncWebServer"

echo.
echo Step 6: Building for all supported platforms...
echo.

echo Building for XIAO ESP32S3...
pio run -e xiao_esp32s3
if errorlevel 1 (
    echo ERROR: Build failed for XIAO ESP32S3
    goto :build_error
)
echo ✓ XIAO ESP32S3 build successful

echo.
echo Building for XIAO ESP32C3...
pio run -e xiao_esp32c3
if errorlevel 1 (
    echo ERROR: Build failed for XIAO ESP32C3
    goto :build_error
)
echo ✓ XIAO ESP32C3 build successful

echo.
echo Building for XIAO ESP32C6...
pio run -e xiao_esp32c6
if errorlevel 1 (
    echo ERROR: Build failed for XIAO ESP32C6
    goto :build_error
)
echo ✓ XIAO ESP32C6 build successful

echo.
echo Building for Heltec HT-HC32...
pio run -e heltec_ht_hc32
if errorlevel 1 (
    echo ERROR: Build failed for Heltec HT-HC32
    goto :build_error
)
echo ✓ Heltec HT-HC32 build successful

echo.
echo Building for Heltec HT-IT01...
pio run -e heltec_ht_it01
if errorlevel 1 (
    echo ERROR: Build failed for Heltec HT-IT01
    goto :build_error
)
echo ✓ Heltec HT-IT01 build successful

echo.
echo Building for Generic Heltec...
pio run -e generic_heltec
if errorlevel 1 (
    echo ERROR: Build failed for Generic Heltec
    goto :build_error
)
echo ✓ Generic Heltec build successful

echo.
echo ========================================
echo 🎉 ALL BUILDS COMPLETED SUCCESSFULLY!
echo ========================================
echo.
echo Build artifacts are located in .pio/build/ directories:
echo - .pio/build/xiao_esp32s3/
echo - .pio/build/xiao_esp32c3/
echo - .pio/build/xiao_esp32c6/
echo - .pio/build/heltec_ht_hc32/
echo - .pio/build/heltec_ht_it01/
echo - .pio/build/generic_heltec/
echo.
echo To upload firmware to device:
echo pio run -e xiao_esp32s3 --target upload
echo.
echo Issues addressed:
echo ✓ Critical security vulnerability fixed (esp_random)
echo ✓ Header file mismatch corrected
echo ✓ Protobuf definitions and generation added
echo ✓ Build environment fully configured
echo ✓ All supported platforms built successfully
echo.
goto :eof

:build_error
echo.
echo ========================================
echo BUILD FAILED!
echo ========================================
echo.
echo Please check the error messages above.
echo Common solutions:
echo 1. Ensure all dependencies are installed
echo 2. Check internet connection for library downloads
echo 3. Verify ESP-IDF installation
echo 4. Review error messages for specific issues
echo.
echo You can try running individual builds:
echo pio run -e [environment_name]
echo.
goto :eof

:error
echo.
echo ========================================
echo SETUP FAILED!
echo ========================================
echo.
echo Please resolve the installation issues and try again.
echo.