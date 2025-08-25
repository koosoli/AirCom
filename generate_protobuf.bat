@echo off
echo ========================================
echo AirCom Protobuf Generation
echo ========================================
echo This script generates protobuf files for AirCom
echo.

REM Check if protoc is available
protoc --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: protoc not found!
    echo Installing protoc via Chocolatey...
    choco install protoc -y
    if errorlevel 1 (
        echo ERROR: Failed to install protoc
        echo Please install protobuf compiler manually
        goto :eof
    )
)

echo Step 1: Generating C++ protobuf files...
cd /d %~dp0

if not exist "components\aircom_proto" (
    mkdir components\aircom_proto
)

echo Generating C++ files...
protoc --cpp_out=components/aircom_proto --proto_path=. AirCom.proto

if errorlevel 1 (
    echo ERROR: Failed to generate protobuf files
    goto :eof
)

echo Step 2: Generating C protobuf files (for ESP-IDF)...
protoc --c_out=components/aircom_proto --proto_path=. AirCom.proto

if errorlevel 1 (
    echo ERROR: Failed to generate C protobuf files
    goto :eof
)

echo Step 3: Renaming generated files for ESP-IDF compatibility...
if exist "components\aircom_proto\AirCom.pb.cc" (
    move "components\aircom_proto\AirCom.pb.cc" "components\aircom_proto\AirCom.pb-c.cpp"
)

if exist "components\aircom_proto\AirCom.pb.h" (
    move "components\aircom_proto\AirCom.pb.h" "components\aircom_proto\AirCom.pb-c.h"
)

echo.
echo ========================================
echo Protobuf generation completed!
echo ========================================
echo.
echo Generated files:
echo - components/aircom_proto/AirCom.pb-c.cpp
echo - components/aircom_proto/AirCom.pb-c.h
echo.