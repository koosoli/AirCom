
# Contributing to AirCom

Thank you for your interest in contributing to AirCom! This document provides guidelines for contributing to this project.

## ⚠️ Important Notice

**This is currently an untested proof-of-concept codebase.** While we welcome contributions, please be aware that:

- The code has not been validated on real hardware
- Significant testing and debugging will be required
- All contributions should be thoroughly tested before submission

## 🚀 Getting Started

### Prerequisites
- PlatformIO IDE or CLI
- ESP32 development environment (ESP-IDF)
- Git for version control
- Basic understanding of ESP32 and FreeRTOS

### Development Setup
1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd AirCom
   ```

2. **Set up development environment**
   ```bash
   # Install PlatformIO if not already installed
   pip install platformio

   # Install ESP-IDF (required for ESP32 development)
   platformio platform install espressif32
   ```

3. **Build and test**
   ```bash
   # Build the project
   platformio run

   # Run GUI preview for testing
   .\simple_gui_preview.bat  # Windows
   # or
   ./simple_gui_preview.sh  # Linux/Mac
   ```

## 📋 Contribution Process

### 1. Fork and Branch
```bash
# Fork the repository on GitHub
# Then create a feature branch
git checkout -b feature/your-feature-name
```

### 2. Development Guidelines

#### Code Standards
- **Language**: C++11 with C compatibility for ESP-IDF
- **Naming**: Use descriptive names following ESP-IDF conventions
- **Documentation**: Document all public functions and complex logic
- **Error Handling**: Use the provided logging system instead of ESP_LOGX
- **Memory Management**: Follow established callback and memory management patterns

#### Code Structure
```
AirCom/
├── main/                    # Main application
│   ├── include/            # Headers
│   ├── *.cpp               # Implementation files
│   └── CMakeLists.txt      # Build configuration
├── components/            # ESP-IDF components
└── [other project files]
```

### 3. Logging System
Use the project's logging system instead of ESP_LOGX:
```cpp
#include "include/logging_system.h"

// Component-specific logging
LOG_INFO("NETWORK", "Network initialized successfully");
LOG_ERROR("AUDIO", ERROR_NONE, "Audio initialization failed");
LOG_DEBUG("UI", "Button pressed: %s", button_name);
```

### 4. Testing Requirements

#### GUI Preview Testing
- Test all interface screens in the GUI preview
- Verify navigation flows
- Test system simulation scenarios
- Check for UI responsiveness

#### Compilation Testing
- Ensure code compiles on all supported platforms
- Test with different hardware configurations
- Verify no new warnings or errors

#### Code Review
- Self-review your code before submission
- Check for memory leaks and resource management
- Verify thread safety considerations

### 5. Commit Guidelines

#### Commit Messages
- Use clear, descriptive commit messages
- Start with a verb (Add, Fix, Update, etc.)
- Keep first line under 50 characters
- Add detailed description for complex changes

**Examples:**
```
Add Bluetooth device discovery functionality
Fix memory leak in audio callback system
Update README with Wi-Fi HaLow configuration
```

#### Commit Structure
```bash
# Make small, focused commits
git add specific-files
git commit -m "Clear commit message"

# Group related changes
git add feature-files
git commit -m "Implement feature X"
```

### 6. Pull Request Process

#### Before Submitting
1. **Test thoroughly** - Ensure no regressions
2. **Update documentation** - Modify README if needed
3. **Check code style** - Follow established patterns
4. **Verify compilation** - Test on supported platforms

#### Pull Request Template
```markdown
## Description
Brief description of the changes

## Changes Made
- List of specific changes
- Files modified
- New features added

## Testing
- How was this tested?
- GUI preview verification
- Compilation testing

## Related Issues
- Link to any related issues
- Reference issue numbers
```

## 🏗️ Architecture Guidelines

### Component Architecture
- Follow the established plugin-based design pattern
- Use interfaces for hardware abstraction
- Implement proper dependency injection

### Thread Safety
- Use established mutex patterns
- Follow callback safety guidelines
- Implement proper synchronization

### Memory Management
- Use the provided SafeCallback system
- Follow established memory tracking patterns
- Avoid manual memory management where possible

### Error Handling
- Use the logging system for all errors
- Follow established error reporting patterns
- Implement proper error recovery

## 🔧 Development Tools

### GUI Preview
```bash
# Test interface without hardware
./simple_gui_preview.bat  # Windows
# or
python gui_preview.py     # Cross-platform
```

### Build System
```bash
# Build for specific platform
platformio run --environment xiao_esp32s3
platformio run --environment heltec_ht_hc32

# Clean build
platformio run --target clean
```

### Code Analysis
- Use ESP-IDF's built-in static analysis tools
- Review code with PlatformIO's linting features
- Check for memory leaks with the built-in tracker

## 📊 Testing Requirements

### Mandatory Testing
- [ ] GUI preview functionality works
- [ ] Code compiles on target platforms
- [ ] No new compiler warnings
- [ ] Documentation updated
- [ ] Error handling implemented

### Recommended Testing
- [ ] Memory leak testing
- [ ] Thread safety verification
- [ ] Performance impact assessment
- [ ] Cross-platform compatibility

## 🤝 Code Review Process

### Review Checklist
- [ ] Code follows established patterns
- [ ] Documentation is comprehensive
- [ ] Error handling is appropriate
- [ ] Thread safety is maintained
- [ ] Memory management is correct
- [ ] Testing is adequate

### Review Comments
- Be constructive and specific
- Suggest improvements, don't just criticize
- Reference coding standards
- Provide examples when possible

## 📞 Getting Help

### Resources
- **ESP-IDF Documentation**: https://docs.espressif.com/projects/esp-idf/
- **PlatformIO Documentation**: https://docs.platformio.org/
- **FreeRTOS Documentation**: https://www.freertos.org/

### Communication
- **Issues**: Use GitHub issues for bugs and feature requests
- **Discussions**: Use GitHub discussions for questions and ideas