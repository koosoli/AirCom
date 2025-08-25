# AirCom Manual Installation Guide

Since the automated scripts are having issues, follow these manual steps to set up your development environment.

## Step 1: Install Python 3.11

### Option A: Download from Python.org (Recommended)
1. Go to https://www.python.org/downloads/
2. Download "Python 3.11.6" for Windows
3. Run the installer: `python-3.11.6-amd64.exe`
4. **IMPORTANT**: Check these options during installation:
   - ✅ "Add python.exe to PATH"
   - ✅ "Install for all users" (optional)
5. Complete the installation

### Option B: Use Microsoft Store
1. Open Microsoft Store
2. Search for "Python"
3. Install "Python 3.11"
4. Python will be automatically added to PATH

### Verify Python Installation
```batch
python --version
# Should show: Python 3.11.x
```

## Step 2: Install PlatformIO

### Method 1: Using pip (Recommended)
```batch
# Install pip if not already installed
python -m ensurepip --upgrade
python -m pip install --upgrade pip

# Install PlatformIO
python -m pip install platformio
```

### Method 2: Using installer (Alternative)
1. Download from: https://platformio.org/install/cli
2. Run the installer
3. Follow the installation wizard

### Verify PlatformIO Installation
```batch
pio --version
# Should show: PlatformIO Core, version x.x.x
```

## Step 3: Install Protocol Buffers (Optional)

### Download protoc compiler
1. Go to https://github.com/protocolbuffers/protobuf/releases
2. Download the latest Windows release (e.g., `protoc-21.12-win64.zip`)
3. Extract the zip file
4. Add the `bin` folder to your system PATH

### Verify protoc installation
```batch
protoc --version
# Should show: libprotoc x.x.x
```

## Step 4: Generate Protocol Buffer Files

```batch
# Navigate to your project directory
cd AirCom

# Generate protobuf files
protoc --c_out=components/aircom_proto --proto_path=. AirCom.proto
```

## Step 5: Build the Project

### Build for specific platform
```batch
# Build for XIAO ESP32S3 (recommended)
pio run -e xiao_esp32s3

# Build for other platforms
pio run -e xiao_esp32c3      # XIAO ESP32C3
pio run -e xiao_esp32c6      # XIAO ESP32C6
pio run -e heltec_ht_hc32    # Heltec HT-HC32
pio run -e heltec_ht_it01    # Heltec HT-IT01
pio run -e generic_heltec    # Generic Heltec
```

### Build all platforms
```batch
# Run each build command separately
pio run -e xiao_esp32s3
pio run -e xiao_esp32c3
pio run -e xiao_esp32c6
pio run -e heltec_ht_hc32
pio run -e heltec_ht_it01
pio run -e generic_heltec
```

## Step 6: Upload to Device (Optional)

```batch
# Upload to connected ESP32 device
pio run -e xiao_esp32s3 --target upload
```

## Troubleshooting

### Python Installation Issues
- Make sure Python is added to your PATH
- Restart your command prompt after installation
- Try `python3` instead of `python`

### PlatformIO Installation Issues
- Use `python -m pip install platformio` instead of just `pip`
- If you have multiple Python versions, use the full path to Python 3.11

### Build Issues
- Ensure you're in the correct directory (`AirCom/`)
- Check that all files are present
- Look for error messages in the build output

### Common Error Messages

**"pio command not found"**
```batch
# Try these solutions:
python -m platformio --version
# or
python3 -m platformio --version
# or
py -3 -m platformio --version
```

**"Python not found"**
- Reinstall Python with "Add to PATH" option checked
- Restart your computer
- Check if Python is in your PATH environment variable

## Verification

After successful installation, you should be able to:

```batch
# Check versions
python --version          # Python 3.11.x
pio --version            # PlatformIO Core x.x.x
protoc --version         # libprotoc x.x.x (if installed)

# Build the project
cd AirCom
pio run -e xiao_esp32s3   # Should complete without errors
```

## Next Steps

Once you can successfully run:
```batch
pio run -e xiao_esp32s3
```

The project is ready for:
1. Hardware testing
2. Further development
3. Production deployment

All critical security issues have been resolved, and the build system is properly configured.