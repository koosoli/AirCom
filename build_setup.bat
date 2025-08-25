@echo off
echo ========================================
echo AirCom - Tactical Communication Setup
echo ========================================
echo.

REM Check if Python is installed
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Python not found!
    echo Please install Python 3.10 or later from https://www.python.org/downloads/
    echo Make sure to check "Add to PATH" during installation.
    goto :error
)

REM Check if PlatformIO is installed
pio --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Installing PlatformIO...
    python -m pip install platformio
    if %errorlevel% neq 0 (
        echo ERROR: Failed to install PlatformIO
        goto :error
    )
)

REM Check if Protocol Buffers compiler is installed
protoc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Protocol Buffers compiler not found.
    echo You may need to install it for full functionality.
    echo Download from: https://github.com/protocolbuffers/protobuf/releases
    echo.
)

echo Generating protocol buffer files...
protoc --c_out=components/aircom_proto --proto_path=. AirCom.proto
if %errorlevel% neq 0 (
    echo WARNING: Protocol buffer generation failed.
    echo You may need to install protoc or generate files manually.
    echo.
) else (
    echo SUCCESS: Protocol buffer files generated.
    echo.
)

echo ========================================
echo Setup completed successfully!
echo.
echo Next steps:
echo 1. Build the firmware: pio run -e xiao_esp32s3
echo 2. Upload to device: pio run -e xiao_esp32s3 --target upload
echo 3. Monitor device: pio run -e xiao_esp32s3 --target monitor
echo.
echo ========================================
goto :eof

:error
echo.
echo ========================================
echo Setup failed! Please resolve the errors above and try again.
echo ========================================
exit /b 1