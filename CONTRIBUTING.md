# Contributing to AirCom

Thank you for your interest in contributing to the AirCom tactical communication firmware! This document outlines the process for contributing to this project.

## 📋 Code of Conduct

By participating in this project, you agree to maintain a respectful and professional environment for all contributors.

## 🚀 Quick Start for Contributors

### Development Environment Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/AirCom.git
   cd AirCom
   ```

2. **Set up development environment**
   ```bash
   # Install PlatformIO
   pip install platformio

   # Generate protocol buffers
   protoc --c_out=components/aircom_proto --proto_path=. AirCom.proto

   # Test build
   pio run -e xiao_esp32s3
   ```

3. **Set up IDE**
   - **VS Code**: Install PlatformIO extension
   - **CLion**: Configure CMake project
   - **Eclipse**: Import as Makefile project

## 🛠️ Development Workflow

### 1. Create Feature Branch
```bash
# Create feature branch
git checkout -b feature/your-feature-name

# Or for bug fixes
git checkout -b bugfix/issue-description
```

### 2. Development Process
- Follow the existing code structure and naming conventions
- Add comprehensive comments for new functions
- Test on multiple ESP32 targets when possible
- Ensure security best practices are maintained

### 3. Code Standards

#### C/C++ Standards
- Use ESP-IDF coding guidelines
- Use meaningful variable and function names
- Add Doxygen-style comments for public APIs
- Handle errors gracefully with proper logging

#### Example Code Structure
```cpp
/**
 * @brief Process incoming tactical data packet
 *
 * This function handles the decryption and validation of incoming
 * tactical communication packets with security verification.
 *
 * @param[in] packet_data Raw packet data buffer
 * @param[in] data_len Length of packet data
 * @param[out] processed_data Decrypted and validated data
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t process_tactical_packet(const uint8_t* packet_data,
                                 size_t data_len,
                                 tactical_data_t* processed_data) {
    ESP_LOGI(TAG, "Processing tactical packet of length %d", data_len);

    // Validate input parameters
    if (packet_data == NULL || processed_data == NULL) {
        ESP_LOGE(TAG, "Invalid input parameters");
        return ESP_ERR_INVALID_ARG;
    }

    // Process packet with security checks
    esp_err_t ret = decrypt_and_validate_packet(packet_data, data_len, processed_data);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Packet processing failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Successfully processed tactical packet");
    return ESP_OK;
}
```

### 4. Testing Requirements

#### Unit Testing
- Test all new functions with various input scenarios
- Include edge cases and error conditions
- Verify security functions work correctly

#### Hardware Testing
- Test on at least one supported ESP32 board
- Verify functionality in real-world conditions
- Test power consumption and performance metrics

#### Security Testing
- Verify cryptographic functions work correctly
- Test secure random number generation
- Validate key exchange mechanisms

### 5. Documentation Updates

#### Code Documentation
- Update function comments with parameter descriptions
- Document any new configuration options
- Update build instructions if needed

#### README Updates
- Add new features to feature list
- Update hardware compatibility matrix
- Include troubleshooting information

## 📝 Pull Request Process

### 1. Pre-Submission Checklist
- [ ] Code compiles successfully on all targets
- [ ] All tests pass
- [ ] Documentation is updated
- [ ] Security review completed
- [ ] Hardware testing completed

### 2. Pull Request Template
Please use the following template for pull requests:

```markdown
## Description
[Brief description of the changes]

## Changes Made
- [List of specific changes]
- [Impact on existing functionality]

## Testing
- [Testing performed]
- [Hardware platforms tested]
- [Security verification]

## Related Issues
- Closes #[issue number]
- Addresses #[issue number]

## Checklist
- [x] Code follows project standards
- [x] Documentation updated
- [x] Tests added/updated
- [x] Security review passed
- [x] Hardware testing completed
```

### 3. Review Process
1. **Automated Checks**: GitHub Actions will run basic validation
2. **Code Review**: At least one maintainer will review the code
3. **Security Review**: Security-critical changes require additional review
4. **Testing Review**: Hardware testing results will be verified

## 🐛 Bug Reports

### Bug Report Template
When reporting bugs, please include:

```markdown
## Bug Description
[Clear description of the issue]

## Steps to Reproduce
1. [Step 1]
2. [Step 2]
3. [Expected behavior]
4. [Actual behavior]

## Environment
- Hardware: [ESP32 board model]
- Firmware version: [commit hash or version]
- PlatformIO version: [version]
- ESP-IDF version: [version]

## Logs
[Relevant log output]
```

## 🔒 Security Considerations

### Security Guidelines
- Never commit sensitive information (keys, credentials, etc.)
- Use secure random number generation
- Implement proper error handling
- Follow cryptographic best practices
- Validate all inputs and outputs

### Security Review Process
- Security-critical changes require additional review
- Use ESP32's hardware security features when possible
- Document security implications of changes

## 📚 Resources

### Documentation
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [Wi-Fi HaLow Specifications](https://www.wi-fi.org/discover-wi-fi/wi-fi-halow)

### Development Tools
- **ESP-IDF VS Code Extension**: Enhanced ESP32 development
- **PlatformIO IDE**: Integrated development environment
- **ESP32 DevKitC**: Primary development board

## 🎯 Feature Requests

### Feature Request Template
```markdown
## Feature Description
[Clear description of the requested feature]

## Use Case
[Why this feature is needed]

## Implementation Approach
[Suggested implementation strategy]

## Impact Assessment
- Security implications: [analysis]
- Performance impact: [analysis]
- Hardware requirements: [analysis]
```

## 📞 Getting Help

- **Issues**: Use GitHub Issues for bugs and feature requests
- **Discussions**: Use GitHub Discussions for general questions
- **Documentation**: Check existing documentation first

## 🙏 Acknowledgments

Thank you for contributing to making tactical communication more secure and reliable!

---

**Happy coding!** 🎯⚡