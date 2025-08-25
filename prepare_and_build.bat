@echo off
echo ========================================
echo AirCom Prepare and Build
echo ========================================
echo Complete build preparation and compilation
echo.

echo Step 1: Check Python installation...
C:\Users\mail\AppData\Local\Programs\Python\Python310\python.exe --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Python not found!
    echo Please install Python 3.11 from https://www.C:\Users\mail\AppData\Local\Programs\Python\Python310\python.exe.org/downloads/
    echo Make sure to check "Add to PATH" during installation
    goto :error
)

echo Step 2: Check PlatformIO installation...
pio --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Installing PlatformIO...
    C:\Users\mail\AppData\Local\Programs\Python\Python310\python.exe -m pip install platformio
    if %errorlevel% neq 0 (
        echo ERROR: Failed to install PlatformIO
        goto :error
    )
)

echo Step 3: Check Protocol Buffers compiler...
C:\Users\mail\.local\bin\protoc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: C:\Users\mail\.local\bin\protoc not found. Skipping protobuf generation.
    echo You may need to install protobuf compiler for full functionality.
    goto :build
)

echo Step 4: Generate protobuf files...
echo Generating C++ protobuf files...
C:\Users\mail\.local\bin\protoc --c_out=components/aircom_proto --proto_path=. AirCom.proto
if %errorlevel% neq 0 (
    echo WARNING: Protobuf generation failed. Continuing anyway...
) else (
    echo Protobuf files generated successfully!
)

:build
echo.
echo Step 5: Build AirCom firmware...
echo Building for XIAO ESP32S3...
pio run -e xiao_esp32s3

if %errorlevel% neq 0 (
    echo ERROR: Build failed for XIAO ESP32S3
    goto :error
)

echo.
echo ========================================
echo SUCCESS! AirCom built successfully! ✅
echo ========================================
echo.
echo Firmware location: .pio\build\xiao_esp32s3\firmware.bin
echo.
echo To upload to your ESP32 device:
echo pio run -e xiao_esp32s3 --target upload
echo.
echo To build for other platforms:
echo pio run -e xiao_esp32c3     # XIAO ESP32C3
echo pio run -e xiao_esp32c6     # XIAO ESP32C6
echo pio run -e heltec_ht_hc32   # Heltec HT-HC32
echo.
goto :eof

:error
echo.
echo ========================================
echo BUILD FAILED! ❌
echo ========================================
echo.
echo Please check the errors above and resolve them.
echo.
echo Quick fixes:
echo 1. Install Python 3.11 from C:\Users\mail\AppData\Local\Programs\Python\Python310\python.exe.org (enable PATH)
echo 2. Run: C:\Users\mail\AppData\Local\Programs\Python\Python310\python.exe -m pip install platformio
echo 3. Try building again: prepare_and_build.bat
echo.