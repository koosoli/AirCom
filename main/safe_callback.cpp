/**
 * @file safe_callback.cpp
 * @brief Safe callback management system implementation
 *
 * This file implements the thread-safe callback system that prevents memory leaks
 * and ensures proper cleanup of callback resources.
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#include "safe_callback.h"
#include "esp_log.h"
#include <algorithm>

static const char* TAG = "SAFE_CALLBACK";

// Static member initialization
std::atomic<uint32_t> CallbackHandle::s_nextId(1);

// CallbackHandle implementation
CallbackHandle::CallbackHandle()
    : m_id(s_nextId.fetch_add(1)), m_valid(true) {
}

CallbackHandle::~CallbackHandle() {
    // Ensure cleanup happens
    if (m_valid) {
        invalidate();
    }
}

void CallbackHandle::invalidate() {
    if (m_valid.exchange(false)) {
        // Notify callback manager of invalidation
        CallbackManager::getInstance().unregisterCallback(this);
    }
}

// CallbackManager implementation
CallbackManager::CallbackManager()
    : m_cleanupCounter(0) {
    m_mutex = xSemaphoreCreateMutex();
    if (!m_mutex) {
        ESP_LOGE(TAG, "Failed to create callback manager mutex");
    }
}

CallbackManager::~CallbackManager() {
    // Cleanup all remaining callbacks
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (auto* callback : m_callbacks) {
            if (callback) {
                callback->m_valid = false;
            }
        }
        m_callbacks.clear();
        xSemaphoreGive(m_mutex);
    }

    if (m_mutex) {
        vSemaphoreDelete(m_mutex);
    }
}

CallbackManager& CallbackManager::getInstance() {
    static CallbackManager instance;
    return instance;
}

void CallbackManager::registerCallback(CallbackHandle* handle) {
    if (!handle) return;

    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Check if already registered
        auto it = std::find(m_callbacks.begin(), m_callbacks.end(), handle);
        if (it == m_callbacks.end()) {
            m_callbacks.push_back(handle);
        }
        xSemaphoreGive(m_mutex);
    }
}

void CallbackManager::unregisterCallback(CallbackHandle* handle) {
    if (!handle) return;

    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        auto it = std::find(m_callbacks.begin(), m_callbacks.end(), handle);
        if (it != m_callbacks.end()) {
            m_callbacks.erase(it);
        }
        xSemaphoreGive(m_mutex);
    }
}

void CallbackManager::invalidateOwnerCallbacks(const std::string& owner) {
    // This is a simplified implementation
    // In a real system, you'd track ownership per callback
    ESP_LOGW(TAG, "Owner-based callback invalidation not fully implemented");
}

size_t CallbackManager::getActiveCallbackCount() const {
    size_t count = 0;
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        count = m_callbacks.size();
        xSemaphoreGive(m_mutex);
    }
    return count;
}

void CallbackManager::cleanupInvalidatedCallbacks() {
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Remove invalidated callbacks
        m_callbacks.erase(
            std::remove_if(m_callbacks.begin(), m_callbacks.end(),
                [](CallbackHandle* handle) {
                    return handle && !handle->isValid();
                }),
            m_callbacks.end());

        xSemaphoreGive(m_mutex);
    }
}

// Factory functions
std::shared_ptr<ConnectionCallback> createConnectionCallback(
    ConnectionCallback::CallbackFunction func,
    const std::string& owner) {

    auto callback = std::make_shared<ConnectionCallback>(std::move(func));
    CallbackManager::getInstance().registerCallback(callback.get());
    return callback;
}

std::shared_ptr<DataCallback> createDataCallback(
    DataCallback::CallbackFunction func,
    const std::string& owner) {

    auto callback = std::make_shared<DataCallback>(std::move(func));
    CallbackManager::getInstance().registerCallback(callback.get());
    return callback;
}

std::shared_ptr<DiscoveryCallback> createDiscoveryCallback(
    DiscoveryCallback::CallbackFunction func,
    const std::string& owner) {

    auto callback = std::make_shared<DiscoveryCallback>(std::move(func));
    CallbackManager::getInstance().registerCallback(callback.get());
    return callback;
}