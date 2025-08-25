#include <chrono>
#include <thread>
#include "Clock.hpp"
#include "Config.hpp"

// High-resolution timer wrapper using standard C++ chrono
static double NowSeconds() {
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start);
    return duration.count() / 1e9;
}

// Function to run headless loop for specified milliseconds and return tick count
extern "C" int RunHeadlessForMs(int ms, int tickHz) {
    // Initialize timing
    double initTime = NowSeconds();
    
    // Create config with specified tick rate
    Config config;
    config.tick_hz = tickHz;
    
    // Initialize clock with config
    Clock clock;
    clock.setTickRate(config.tick_hz);
    
    double last = NowSeconds();
    double startTime = last;
    
    // Headless engine loop
    while (true) {
        double now = NowSeconds(); 
        double frame = now - last; 
        last = now;
        
        // Check if we should exit based on ms limit
        double elapsedMs = (now - startTime) * 1000.0;
        if (elapsedMs >= ms) {
            break;
        }
        
        clock.step(frame); // Fixed-step simulation
        
        // Small sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    return static_cast<int>(clock.ticks);
}
