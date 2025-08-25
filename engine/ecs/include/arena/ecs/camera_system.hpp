#pragma once
#include "arena/ecs/registry.hpp"
#include "arena/ecs/components.hpp"
#include "arena/input.hpp"

namespace arena::ecs {

class CameraSystem {
public:
    // Update camera entities based on input state
    void update(float dt, const arena::InputState& input, Registry& registry);
    
private:
    // Helper function to clamp pitch to ±89 degrees
    static float clampPitch(float pitch);
};

} // namespace arena::ecs
