#include <iostream>
#include <cassert>
#include <unordered_map>
#include "../src/app/Config.hpp"

// Simple test macros without Catch2
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "FAILED: " << message << std::endl; \
            std::cerr << "  at line " << __LINE__ << std::endl; \
            return false; \
        } else { \
            std::cout << "PASSED: " << message << std::endl; \
        } \
    } while(0)

bool test_config_defaults() {
    std::cout << "\n=== Testing Config defaults ===" << std::endl;
    
    Config config;
    
    TEST_ASSERT(config.tick_hz == 60, "Default tick_hz should be 60");
    TEST_ASSERT(config.window_w == 1280, "Default window_w should be 1280");
    TEST_ASSERT(config.window_h == 720, "Default window_h should be 720");
    TEST_ASSERT(config.getTickDelta() == 1.0/60.0, "Default tick delta should be 1/60");
    
    return true;
}

bool test_config_overrides() {
    std::cout << "\n=== Testing Config overrides ===" << std::endl;
    
    Config config;
    
    // Test with key-value pairs as specified in requirements
    auto kv = std::unordered_map<std::string, std::string>{
        {"tick_hz", "120"},
        {"window_w", "640"}
    };
    
    config.loadFromMap(kv);
    
    TEST_ASSERT(config.tick_hz == 120, "tick_hz should be overridden to 120");
    TEST_ASSERT(config.window_w == 640, "window_w should be overridden to 640");
    TEST_ASSERT(config.window_h == 720, "window_h should remain default 720");
    TEST_ASSERT(config.getTickDelta() == 1.0/120.0, "tick delta should be 1/120");
    
    return true;
}

bool test_config_invalid_values() {
    std::cout << "\n=== Testing Config invalid values ===" << std::endl;
    
    Config config;
    
    // Test with invalid values
    auto kv = std::unordered_map<std::string, std::string>{
        {"tick_hz", "invalid"},
        {"window_w", "-100"},
        {"window_h", "0"}
    };
    
    config.loadFromMap(kv);
    
    // Should fall back to defaults
    TEST_ASSERT(config.tick_hz == 60, "Invalid tick_hz should fall back to 60");
    TEST_ASSERT(config.window_w == 1280, "Invalid window_w should fall back to 1280");
    TEST_ASSERT(config.window_h == 720, "Invalid window_h should fall back to 720");
    
    return true;
}

bool test_config_mixed_valid_invalid() {
    std::cout << "\n=== Testing Config mixed valid/invalid values ===" << std::endl;
    
    Config config;
    
    // Test with mix of valid and invalid values
    auto kv = std::unordered_map<std::string, std::string>{
        {"tick_hz", "90"},
        {"window_w", "1920"},
        {"window_h", "invalid"}
    };
    
    config.loadFromMap(kv);
    
    TEST_ASSERT(config.tick_hz == 90, "Valid tick_hz should be 90");
    TEST_ASSERT(config.window_w == 1920, "Valid window_w should be 1920");
    TEST_ASSERT(config.window_h == 720, "Invalid window_h should fall back to 720");
    
    return true;
}

int main() {
    std::cout << "Arena Engine - Config Tests" << std::endl;
    std::cout << "===========================" << std::endl;
    
    int passed = 0;
    int total = 0;
    
    // Run all tests
    ++total; if (test_config_defaults()) ++passed;
    ++total; if (test_config_overrides()) ++passed;
    ++total; if (test_config_invalid_values()) ++passed;
    ++total; if (test_config_mixed_valid_invalid()) ++passed;
    
    std::cout << "\n===========================" << std::endl;
    std::cout << "Test Results: " << passed << "/" << total << " tests passed" << std::endl;
    
    if (passed == total) {
        std::cout << "All tests PASSED! 🎉" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests FAILED! ❌" << std::endl;
        return 1;
    }
}
