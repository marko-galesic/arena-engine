#pragma once
#include <string>
#include <unordered_map>

struct Config {
    // Core engine options
    int tick_hz = 60;
    int window_w = 1280;
    int window_h = 720;
    
    // Load configuration from INI file
    bool loadFromFile(const std::string& path);
    
    // Load configuration from key-value pairs (for testing)
    void loadFromMap(const std::unordered_map<std::string, std::string>& kv);
    
    // Get tick delta time in seconds
    double getTickDelta() const { return 1.0 / tick_hz; }
};

// Simple INI loader function
std::unordered_map<std::string, std::string> LoadIni(const char* path);
