#include "Config.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cctype>

std::unordered_map<std::string, std::string> LoadIni(const char* path) {
    std::unordered_map<std::string, std::string> kv;
    FILE* f = fopen(path, "rb");
    if (!f) return kv;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == ';') continue;
        
        char* eq = strchr(line, '=');
        if (!eq) continue;
        
        *eq = 0;
        std::string k = line;
        std::string v = eq + 1;
        
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
            s.erase(s.find_last_not_of(" \t\r\n") + 1);
        };
        
        trim(k);
        trim(v);
        kv[k] = v;
    }
    
    fclose(f);
    return kv;
}

bool Config::loadFromFile(const std::string& path) {
    auto kv = LoadIni(path.c_str());
    if (kv.empty()) return false;
    
    loadFromMap(kv);
    return true;
}

void Config::loadFromMap(const std::unordered_map<std::string, std::string>& kv) {
    // Parse tick_hz
    auto it = kv.find("tick_hz");
    if (it != kv.end()) {
        try {
            tick_hz = std::stoi(it->second);
            if (tick_hz <= 0) tick_hz = 60; // Default to 60 if invalid
        } catch (...) {
            tick_hz = 60; // Default to 60 if parsing fails
        }
    }
    
    // Parse window_w
    it = kv.find("window_w");
    if (it != kv.end()) {
        try {
            window_w = std::stoi(it->second);
            if (window_w <= 0) window_w = 1280; // Default to 1280 if invalid
        } catch (...) {
            window_w = 1280; // Default to 1280 if parsing fails
        }
    }
    
    // Parse window_h
    it = kv.find("window_h");
    if (it != kv.end()) {
        try {
            window_h = std::stoi(it->second);
            if (window_h <= 0) window_h = 720; // Default to 720 if invalid
        } catch (...) {
            window_h = 720; // Default to 720 if parsing fails
        }
    }
}
