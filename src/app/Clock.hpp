#pragma once
#include <cstdint>

struct Clock {
    // Fixed-step accumulator clock (seconds)
    double accumulator = 0.0;
    double dt = 1.0/60.0;
    uint64_t ticks = 0;
    
    // Set the tick rate (updates dt)
    void setTickRate(int hz) {
        if (hz > 0) {
            dt = 1.0 / hz;
        }
    }
    
    void step(double frameSeconds) {
        accumulator += frameSeconds;
        while (accumulator + 1e-12 >= dt) { 
            accumulator -= dt; 
            ++ticks; 
        }
    }
};
