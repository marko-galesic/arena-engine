#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <windows.h>
#include "app/Clock.hpp"
#include "app/Config.hpp"

// High-resolution timer wrapper using QueryPerformanceCounter
static double NowSeconds() {
    static LARGE_INTEGER f; 
    static bool init = false;
    if (!init) { 
        QueryPerformanceFrequency(&f); 
        init = true; 
    }
    LARGE_INTEGER c; 
    QueryPerformanceCounter(&c);
    return double(c.QuadPart) / double(f.QuadPart);
}

// Simple logging macro with timestamps
#define LOG(msg) std::cout << "[" << NowSeconds() << "] " << msg << std::endl

// Simple command line argument parser
struct Args {
    bool server = false;
    std::string configPath = "config/engine.ini";
    int runForMs = -1; // -1 means run indefinitely
    
    void parse(int argc, char* argv[]) {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "--server") {
                server = true;
            } else if (arg.substr(0, 9) == "--config=") {
                configPath = arg.substr(9);
            } else if (arg.substr(0, 11) == "--runForMs=") {
                try {
                    runForMs = std::stoi(arg.substr(11));
                } catch (...) {
                    std::cout << "Warning: Invalid --runForMs value, ignoring" << std::endl;
                }
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "Arena Engine\n";
                std::cout << "Usage: arena [options]\n";
                std::cout << "Options:\n";
                std::cout << "  --server              Run in headless mode (no window)\n";
                std::cout << "  --config=<path>       Load configuration from file\n";
                std::cout << "  --runForMs=<ms>       Run for specified milliseconds then exit\n";
                std::cout << "  --help, -h            Show this help message\n";
                exit(0);
            }
        }
    }
};

int main(int argc, char* argv[]) {
    // Initialize timing first
    double initTime = NowSeconds();
    
    Args args;
    args.parse(argc, argv);
    
    LOG("Starting Arena Engine");
    if (args.server) {
        LOG("Running in HEADLESS mode (--server)");
    } else {
        LOG("Running in WINDOWED mode");
    }
    
    // Load configuration
    Config config;
    if (config.loadFromFile(args.configPath)) {
        LOG("Loaded config from: " << args.configPath);
        LOG("Tick rate: " << config.tick_hz << " Hz");
        LOG("Window size: " << config.window_w << "x" << config.window_h);
    } else {
        LOG("Warning: Could not load config from " << args.configPath << ", using defaults");
    }
    
    // Initialize clock with config
    Clock clock;
    clock.setTickRate(config.tick_hz);
    
    double last = NowSeconds();
    double lastLogTime = last;
    double startTime = last;
    
    LOG("Engine loop starting...");
    if (args.runForMs > 0) {
        LOG("Will run for " << args.runForMs << "ms then exit");
    }
    
    // Main engine loop
    while (true) {
        double now = NowSeconds(); 
        double frame = now - last; 
        last = now;
        
        // Check if we should exit based on --runForMs FIRST (before anything else)
        if (args.runForMs > 0) {
            double elapsedMs = (now - startTime) * 1000.0;
            if (elapsedMs >= args.runForMs) {
                LOG("Reached --runForMs limit (" << args.runForMs << "ms), exiting");
                break;
            }
        }
        
        clock.step(frame); // Fixed-step simulation
        
        // Log tick count every second
        if (now - lastLogTime >= 1.0) {
            double actualRate = clock.ticks / (now - startTime);
            LOG("Ticks: " << clock.ticks << " (accum: " << clock.accumulator << ", rate: " << actualRate << " Hz, target: " << config.tick_hz << " Hz)");
            lastLogTime = now;
        }
        
        // Small sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    double totalTime = NowSeconds() - startTime;
    double finalRate = clock.ticks / totalTime;
    
    LOG("Final Results:");
    LOG("Total time: " << totalTime << " seconds");
    LOG("Total ticks: " << clock.ticks);
    LOG("Average rate: " << finalRate << " Hz (target: " << config.tick_hz << " Hz)");
    LOG("Rate accuracy: " << (finalRate / config.tick_hz * 100.0) << "%");
    
    return 0;
}
