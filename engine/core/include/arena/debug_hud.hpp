#pragma once

// Forward declarations
struct Clock;

namespace arena {

struct InputState;

class DebugHud {
public:
    // Draw the debug HUD overlay
    static void draw(const Clock& clock, const InputState& input);
    
private:
    // Helper function to check if a key is pressed
    static bool isKeyPressed(const InputState& input, int key);
};

} // namespace arena
