/**
 * @file safe_callback.h
 * @brief Safe callback management system for AirCom
 *
 * This file provides a thread-safe callback system that prevents memory leaks
 * and ensures proper cleanup of callback resources.
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#ifndef SAFE_CALLBACK_H
#define SAFE_CALLBACK_H

#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <atomic>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class CallbackManager;

/**
 * @brief Base callback handle for lifetime management
 */
class CallbackHandle {
public:
    CallbackHandle();
    virtual ~CallbackHandle();

    /**
     * @brief Check if callback is still valid
     * @return true if valid, false if invalidated
     */
    bool isValid() const { return m_valid; }

    /**
     * @brief Invalidate the callback handle
     */
    void invalidate();

    /**
     * @brief Get unique callback ID
     * @return Callback identifier
     */
    uint32_t getId() const { return m_id; }

private:
    uint32_t m_id;
    std::atomic<bool> m_valid;
    static std::atomic<uint32_t> s_nextId;

    friend class CallbackManager;
};

/**
 * @brief Type-safe callback wrapper
 */
template<typename... Args>
class SafeCallback : public CallbackHandle {
public:
    using CallbackFunction = std::function<void(Args...)>;

    SafeCallback(CallbackFunction func)
        : m_callback(std::move(func)) {}

    /**
     * @brief Execute the callback if still valid
     * @param args Arguments to pass to callback
     * @return true if callback was executed, false if invalid
     */
    bool execute(Args... args) {
        if (!isValid()) {
            return false;
        }

        try {
            m_callback(std::forward<Args>(args)...);
            return true;
        } catch (const std::exception& e) {
            // Log error but don't crash
            ESP_LOGE("SafeCallback", "Callback execution failed: %s", e.what());
            return false;
        }
    }

    /**
     * @brief Check if callback is set
     * @return true if callback function is set
     */
    bool isSet() const {
        return static_cast<bool>(m_callback);
    }

private:
    CallbackFunction m_callback;
};

/**
 * @brief Callback manager for centralized callback lifecycle management
 */
class CallbackManager {
public:
    static CallbackManager& getInstance();

    /**
     * @brief Register a callback handle for tracking
     * @param handle Callback handle to register
     */
    void registerCallback(CallbackHandle* handle);

    /**
     * @brief Unregister a callback handle
     * @param handle Callback handle to unregister
     */
    void unregisterCallback(CallbackHandle* handle);

    /**
     * @brief Invalidate all callbacks for a specific owner
     * @param owner Owner identifier
     */
    void invalidateOwnerCallbacks(const std::string& owner);

    /**
     * @brief Get callback statistics
     * @return Number of active callbacks
     */
    size_t getActiveCallbackCount() const;

    /**
     * @brief Cleanup invalidated callbacks
     */
    void cleanupInvalidatedCallbacks();

private:
    CallbackManager();
    ~CallbackManager();

    // Prevent copying
    CallbackManager(const CallbackManager&) = delete;
    CallbackManager& operator=(const CallbackManager&) = delete;

    mutable SemaphoreHandle_t m_mutex;
    std::vector<CallbackHandle*> m_callbacks;
    std::atomic<size_t> m_cleanupCounter;
};

/**
 * @brief Connection event callback type
 */
using ConnectionCallback = SafeCallback<const std::string&, bool>;

/**
 * @brief Data event callback type
 */
using DataCallback = SafeCallback<const std::string&, const std::vector<uint8_t>&>;

/**
 * @brief Discovery event callback type
 */
using DiscoveryCallback = SafeCallback<const std::vector<std::string>&>;

/**
 * @brief Generic event callback type
 */
using EventCallback = SafeCallback<const std::string&, void*>;

/**
 * @brief Helper class for automatic callback registration
 */
class CallbackOwner {
public:
    CallbackOwner(const std::string& ownerId)
        : m_ownerId(ownerId) {}

    ~CallbackOwner() {
        // Invalidate all callbacks for this owner
        CallbackManager::getInstance().invalidateOwnerCallbacks(m_ownerId);
    }

    /**
     * @brief Get owner identifier
     * @return Owner ID
     */
    const std::string& getOwnerId() const { return m_ownerId; }

private:
    std::string m_ownerId;
};

/**
 * @brief RAII wrapper for callback handles
 */
template<typename T>
class ScopedCallback {
public:
    ScopedCallback(std::shared_ptr<T> callback)
        : m_callback(callback) {}

    ~ScopedCallback() {
        if (m_callback) {
            m_callback->invalidate();
        }
    }

    /**
     * @brief Get the callback pointer
     * @return Shared pointer to callback
     */
    std::shared_ptr<T> get() const { return m_callback; }

    /**
     * @brief Check if callback is valid
     * @return true if valid
     */
    bool isValid() const {
        return m_callback && m_callback->isValid();
    }

private:
    std::shared_ptr<T> m_callback;
};

/**
 * @brief Create a connection callback with automatic registration
 */
std::shared_ptr<ConnectionCallback> createConnectionCallback(
    ConnectionCallback::CallbackFunction func,
    const std::string& owner = "");

/**
 * @brief Create a data callback with automatic registration
 */
std::shared_ptr<DataCallback> createDataCallback(
    DataCallback::CallbackFunction func,
    const std::string& owner = "");

/**
 * @brief Create a discovery callback with automatic registration
 */
std::shared_ptr<DiscoveryCallback> createDiscoveryCallback(
    DiscoveryCallback::CallbackFunction func,
    const std::string& owner = "");

#endif // SAFE_CALLBACK_H