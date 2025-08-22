/**
 * @file halow_factory.h
 * @brief Factory implementation for creating Wi-Fi HaLow implementations
 *
 * This file implements the HaLowFactory class that creates appropriate
 * Wi-Fi HaLow implementations based on hardware detection and user preferences.
 *
 * @author AirCom Development Team
 * @version 2.0.0
 * @date 2024
 */

#ifndef HALOW_FACTORY_H
#define HALOW_FACTORY_H

#include "halow_interface.h"
#include <memory>
#include <map>
#include <string>

/**
 * @brief Implementation registry entry
 */
struct ImplementationEntry {
    std::string name;
    std::string description;
    std::vector<std::string> supported_hardware;
    uint32_t priority; // Higher priority = preferred choice
    bool requires_sdk; // Whether external SDK is needed
    std::string sdk_url; // URL to SDK or documentation
};

/**
 * @brief HaLowFactory implementation
 */
class HaLowFactory {
public:
    /**
     * @brief Get the singleton instance
     */
    static HaLowFactory& getInstance();

    /**
     * @brief Create Wi-Fi HaLow implementation based on hardware and preferences
     * @param hardware_type Target hardware type (auto-detected if empty)
     * @param preferred_sdk Preferred SDK implementation
     * @return Unique pointer to HaLow implementation
     */
    std::unique_ptr<IHaLow> createHaLow(const std::string& hardware_type = "",
                                       const std::string& preferred_sdk = "");

    /**
     * @brief Create optimized HaLow implementation for current hardware
     * @return Unique pointer to best HaLow implementation for current hardware
     */
    std::unique_ptr<IHaLow> createOptimalHaLow();

    /**
     * @brief Get list of available implementations
     * @return Vector of implementation names
     */
    std::vector<std::string> getAvailableImplementations();

    /**
     * @brief Get implementations supported by specific hardware
     * @param hardware_type Hardware type to check
     * @return Vector of implementation names
     */
    std::vector<std::string> getSupportedImplementations(const std::string& hardware_type);

    /**
     * @brief Check if hardware is supported by any implementation
     * @param hardware_type Hardware type to check
     * @return true if supported, false otherwise
     */
    bool isHardwareSupported(const std::string& hardware_type);

    /**
     * @brief Get implementation details
     * @param implementation_name Name of implementation
     * @return Implementation entry or nullptr if not found
     */
    const ImplementationEntry* getImplementationDetails(const std::string& implementation_name);

    /**
     * @brief Register a new HaLow implementation
     * @param entry Implementation entry to register
     * @return true on success, false on failure
     */
    bool registerImplementation(const ImplementationEntry& entry);

    /**
     * @brief Unregister a HaLow implementation
     * @param implementation_name Name of implementation to remove
     * @return true on success, false on failure
     */
    bool unregisterImplementation(const std::string& implementation_name);

    /**
     * @brief Auto-detect hardware type
     * @return Detected hardware type string
     */
    std::string autoDetectHardware();

    /**
     * @brief Get recommended implementation for hardware
     * @param hardware_type Hardware type
     * @return Recommended implementation name
     */
    std::string getRecommendedImplementation(const std::string& hardware_type);

    /**
     * @brief Test implementation compatibility
     * @param implementation Implementation to test
     * @param hardware_type Hardware type to test against
     * @return Compatibility score (0-100, higher is better)
     */
    int testCompatibility(const std::string& implementation, const std::string& hardware_type);

private:
    // Private constructor for singleton
    HaLowFactory();
    ~HaLowFactory();

    // Prevent copying
    HaLowFactory(const HaLowFactory&) = delete;
    HaLowFactory& operator=(const HaLowFactory&) = delete;

    // Implementation registry
    std::map<std::string, ImplementationEntry> m_implementations;

    // Hardware detection cache
    std::string m_detectedHardware;

    // Initialize built-in implementations
    void initializeBuiltInImplementations();

    // Create specific implementation types
    std::unique_ptr<IHaLow> createMMIoTSDKHaLow();
    std::unique_ptr<IHaLow> createHeltecHaLow();
    std::unique_ptr<IHaLow> createEspIdfHaLow();
    std::unique_ptr<IHaLow> createGenericHaLow();

    // Hardware-specific factory methods
    std::unique_ptr<IHaLow> createForXiaoESP32S3();
    std::unique_ptr<IHaLow> createForXiaoESP32C3();
    std::unique_ptr<IHaLow> createForXiaoESP32C6();
    std::unique_ptr<IHaLow> createForHeltecHTHC32();
    std::unique_ptr<IHaLow> createForHeltecHTIT01();
    std::unique_ptr<IHaLow> createForGenericHeltec();
    std::unique_ptr<IHaLow> createForGenericESP32();

    // Configuration-based creation
    std::unique_ptr<IHaLow> createFromConfig(const std::string& config_name);
};

/**
 * @brief Hardware compatibility matrix
 */
struct HardwareCompatibility {
    std::string hardware_type;
    std::vector<std::string> compatible_implementations;
    std::vector<std::string> recommended_implementations;
    std::string notes;
};

/**
 * @brief Get hardware compatibility matrix
 * @return Vector of hardware compatibility entries
 */
std::vector<HardwareCompatibility> getHardwareCompatibilityMatrix();

#endif // HALOW_FACTORY_H