#pragma once

namespace arena {

struct InputState {
    bool keys[512];
    double mouseDx, mouseDy;
    bool mouseButtons[8];
};

// Clear deltas at the beginning of each frame
void beginFrame(InputState& state);

// Handle GLFW key events
void handleKey(InputState& state, int key, int action);

// Handle mouse movement with dead-zone clamping
void handleMouseMove(InputState& state, double dx, double dy);

// Handle mouse button events
void handleMouseButton(InputState& state, int button, int action);

} // namespace arena
