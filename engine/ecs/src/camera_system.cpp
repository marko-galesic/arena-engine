#include "arena/ecs/camera_system.hpp"
#include "arena/ecs/components.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// GLFW key constants (copied to avoid dependency)
#define GLFW_KEY_W 87
#define GLFW_KEY_A 65
#define GLFW_KEY_S 83
#define GLFW_KEY_D 68
#define GLFW_KEY_SPACE 32
#define GLFW_KEY_C 67

namespace arena::ecs {

void CameraSystem::update(float dt, const arena::InputState& input, Registry& registry) {
    // Process all entities with both Transform and CameraController components
    registry.view<Transform, CameraController>([&](Entity entity, Transform& transform, CameraController& controller) {
        // Handle keyboard movement
        float moveSpeed = controller.moveSpeed * dt;
        
        // Forward/backward movement (W/S keys)
        if (input.keys[GLFW_KEY_W]) {
            // Move forward in the direction the camera is facing
            float yaw = transform.rotYawPitchRoll[0];
            transform.pos[0] += std::sin(yaw) * moveSpeed;
            transform.pos[2] -= std::cos(yaw) * moveSpeed;
        }
        if (input.keys[GLFW_KEY_S]) {
            // Move backward
            float yaw = transform.rotYawPitchRoll[0];
            transform.pos[0] -= std::sin(yaw) * moveSpeed;
            transform.pos[2] += std::cos(yaw) * moveSpeed;
        }
        
        // Left/right strafe movement (A/D keys)
        if (input.keys[GLFW_KEY_A]) {
            // Strafe left (perpendicular to forward direction)
            float yaw = transform.rotYawPitchRoll[0];
            transform.pos[0] -= std::cos(yaw) * moveSpeed;
            transform.pos[2] -= std::sin(yaw) * moveSpeed;
        }
        if (input.keys[GLFW_KEY_D]) {
            // Strafe right
            float yaw = transform.rotYawPitchRoll[0];
            transform.pos[0] += std::cos(yaw) * moveSpeed;
            transform.pos[2] += std::sin(yaw) * moveSpeed;
        }
        
        // Up/down movement (Space/C keys)
        if (input.keys[GLFW_KEY_SPACE]) {
            transform.pos[1] += moveSpeed;
        }
        if (input.keys[GLFW_KEY_C]) {
            transform.pos[1] -= moveSpeed;
        }
        
        // Handle mouse look
        float lookSensitivity = controller.lookSensitivity;
        
        // Yaw rotation (left/right mouse movement)
        if (input.mouseDx != 0.0) {
            transform.rotYawPitchRoll[0] += input.mouseDx * lookSensitivity;
        }
        
        // Pitch rotation (up/down mouse movement)
        if (input.mouseDy != 0.0) {
            float newPitch = transform.rotYawPitchRoll[1] - input.mouseDy * lookSensitivity;
            transform.rotYawPitchRoll[1] = clampPitch(newPitch);
        }
    });
}

float CameraSystem::clampPitch(float pitch) {
    const float maxPitch = 89.0f * M_PI / 180.0f; // Convert 89 degrees to radians
    return std::clamp(pitch, -maxPitch, maxPitch);
}

} // namespace arena::ecs
