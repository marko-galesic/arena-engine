#include "arena/input.hpp"
#include <cstring>
#include <cmath>

namespace arena {

void beginFrame(InputState& state) {
    // Clear mouse deltas for the new frame
    state.mouseDx = 0.0;
    state.mouseDy = 0.0;
}

void handleKey(InputState& state, int key, int action) {
    if (key >= 0 && key < 512) {
        // GLFW_PRESS = 1, GLFW_RELEASE = 0, GLFW_REPEAT = 2
        state.keys[key] = (action == 1); // true for press, false for release/repeat
    }
}

void handleMouseMove(InputState& state, double dx, double dy) {
    // Apply dead-zone: clamp values < 0.001 to 0
    const double deadZone = 0.001;
    
    if (std::abs(dx) < deadZone) dx = 0.0;
    if (std::abs(dy) < deadZone) dy = 0.0;
    
    state.mouseDx = dx;
    state.mouseDy = dy;
}

void handleMouseButton(InputState& state, int button, int action) {
    if (button >= 0 && button < 8) {
        // GLFW_PRESS = 1, GLFW_RELEASE = 0
        state.mouseButtons[button] = (action == 1); // true for press, false for release
    }
}

} // namespace arena
