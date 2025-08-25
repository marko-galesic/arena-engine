#include <catch2/catch_test_macros.hpp>
#include "arena/ecs/registry.hpp"
#include "arena/ecs/camera_system.hpp"
#include "arena/input.hpp"
#include <GLFW/glfw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TEST_CASE("CameraSystem basic functionality", "[camera]") {
    arena::ecs::Registry registry;
    arena::ecs::CameraSystem cameraSystem;
    
    // Create a camera entity
    arena::ecs::Entity camera = registry.create();
    registry.add<arena::ecs::Transform>(camera, {{0, 0, 5}, {0, 0, 0}, {1, 1, 1}});
    registry.add<arena::ecs::CameraController>(camera, {5.0f, 0.002f});
    
    // Get references to components
    auto* transform = registry.get<arena::ecs::Transform>(camera);
    auto* controller = registry.get<arena::ecs::CameraController>(camera);
    
    REQUIRE(transform != nullptr);
    REQUIRE(controller != nullptr);
    REQUIRE(transform->pos[0] == 0.0f);
    REQUIRE(transform->pos[1] == 0.0f);
    REQUIRE(transform->pos[2] == 5.0f);
    REQUIRE(transform->rotYawPitchRoll[0] == 0.0f);
    REQUIRE(transform->rotYawPitchRoll[1] == 0.0f);
    REQUIRE(transform->rotYawPitchRoll[2] == 0.0f);
}

TEST_CASE("CameraSystem forward movement (W key)", "[camera]") {
    arena::ecs::Registry registry;
    arena::ecs::CameraSystem cameraSystem;
    
    // Create a camera entity
    arena::ecs::Entity camera = registry.create();
    registry.add<arena::ecs::Transform>(camera, {{0, 0, 5}, {0, 0, 0}, {1, 1, 1}});
    registry.add<arena::ecs::CameraController>(camera, {5.0f, 0.002f});
    
    auto* transform = registry.get<arena::ecs::Transform>(camera);
    
    // Create input state with W key held
    arena::InputState input = {};
    input.keys[GLFW_KEY_W] = true;
    
    // Simulate 1 second at 60Hz (60 frames)
    const float dt = 1.0f / 60.0f;
    const int frames = 60;
    
    float initialZ = transform->pos[2];
    
    for (int i = 0; i < frames; ++i) {
        cameraSystem.update(dt, input, registry);
    }
    
    // Check that Z position increased (moved forward)
    REQUIRE(transform->pos[2] < initialZ);
    
    // Calculate expected movement: 5.0f * (1.0f/60.0f) * 60 = 5.0f
    float expectedMovement = 5.0f * dt * frames;
    float actualMovement = initialZ - transform->pos[2];
    
    // Allow small floating point precision differences
    REQUIRE(std::abs(actualMovement - expectedMovement) < 0.001f);
}

TEST_CASE("CameraSystem yaw rotation with mouse", "[camera]") {
    arena::ecs::Registry registry;
    arena::ecs::CameraSystem cameraSystem;
    
    // Create a camera entity
    arena::ecs::Entity camera = registry.create();
    registry.add<arena::ecs::Transform>(camera, {{0, 0, 5}, {0, 0, 0}, {1, 1, 1}});
    registry.add<arena::ecs::CameraController>(camera, {5.0f, 0.002f});
    
    auto* transform = registry.get<arena::ecs::Transform>(camera);
    
    // Create input state with mouse movement
    arena::InputState input = {};
    input.mouseDx = 10.0; // 10 pixels right
    
    const float dt = 1.0f / 60.0f;
    
    float initialYaw = transform->rotYawPitchRoll[0];
    
    // Update once
    cameraSystem.update(dt, input, registry);
    
    // Check that yaw increased
    REQUIRE(transform->rotYawPitchRoll[0] > initialYaw);
    
    // Calculate expected yaw change: 10.0 * 0.002f = 0.02f
    float expectedYawChange = 10.0 * 0.002f;
    float actualYawChange = transform->rotYawPitchRoll[0] - initialYaw;
    
    REQUIRE(std::abs(actualYawChange - expectedYawChange) < 0.001f);
}

TEST_CASE("CameraSystem pitch rotation with mouse", "[camera]") {
    arena::ecs::Registry registry;
    arena::ecs::CameraSystem cameraSystem;
    
    // Create a camera entity
    arena::ecs::Entity camera = registry.create();
    registry.add<arena::ecs::Transform>(camera, {{0, 0, 5}, {0, 0, 0}, {1, 1, 1}});
    registry.add<arena::ecs::CameraController>(camera, {5.0f, 0.002f});
    
    auto* transform = registry.get<arena::ecs::Transform>(camera);
    
    // Create input state with mouse movement
    arena::InputState input = {};
    input.mouseDy = 15.0; // 15 pixels down
    
    const float dt = 1.0f / 60.0f;
    
    float initialPitch = transform->rotYawPitchRoll[1];
    
    // Update once
    cameraSystem.update(dt, input, registry);
    
    // Check that pitch decreased (mouse down = look down)
    REQUIRE(transform->rotYawPitchRoll[1] < initialPitch);
    
    // Calculate expected pitch change: 15.0 * 0.002f = 0.03f
    float expectedPitchChange = 15.0 * 0.002f;
    float actualPitchChange = initialPitch - transform->rotYawPitchRoll[1];
    
    REQUIRE(std::abs(actualPitchChange - expectedPitchChange) < 0.001f);
}

TEST_CASE("CameraSystem pitch clamping", "[camera]") {
    arena::ecs::Registry registry;
    arena::ecs::CameraSystem cameraSystem;
    
    // Create a camera entity
    arena::ecs::Entity camera = registry.create();
    registry.add<arena::ecs::Transform>(camera, {{0, 0, 5}, {0, 0, 0}, {1, 1, 1}});
    registry.add<arena::ecs::CameraController>(camera, {5.0f, 0.002f});
    
    auto* transform = registry.get<arena::ecs::Transform>(camera);
    
    // Create input state with extreme mouse movement
    arena::InputState input = {};
    input.mouseDy = 10000.0; // Very large movement
    
    const float dt = 1.0f / 60.0f;
    
    // Update many times to try to exceed pitch limits
    for (int i = 0; i < 1000; ++i) {
        cameraSystem.update(dt, input, registry);
    }
    
    // Pitch should be clamped to approximately ±89 degrees (±1.553 radians)
    const float maxPitch = 89.0f * M_PI / 180.0f;
    REQUIRE(transform->rotYawPitchRoll[1] <= maxPitch);
    REQUIRE(transform->rotYawPitchRoll[1] >= -maxPitch);
}

TEST_CASE("CameraSystem strafe movement (A/D keys)", "[camera]") {
    arena::ecs::Registry registry;
    arena::ecs::CameraSystem cameraSystem;
    
    // Create a camera entity
    arena::ecs::Entity camera = registry.create();
    registry.add<arena::ecs::Transform>(camera, {{0, 0, 5}, {0, 0, 0}, {1, 1, 1}});
    registry.add<arena::ecs::CameraController>(camera, {5.0f, 0.002f});
    
    auto* transform = registry.get<arena::ecs::Transform>(camera);
    
    // Create input state with A key held (strafe left)
    arena::InputState input = {};
    input.keys[GLFW_KEY_A] = true;
    
    const float dt = 1.0f / 60.0f;
    const int frames = 60;
    
    float initialX = transform->pos[0];
    
    for (int i = 0; i < frames; ++i) {
        cameraSystem.update(dt, input, registry);
    }
    
    // Check that X position decreased (moved left)
    REQUIRE(transform->pos[0] < initialX);
    
    // Calculate expected movement: 5.0f * (1.0f/60.0f) * 60 = 5.0f
    float expectedMovement = 5.0f * dt * frames;
    float actualMovement = initialX - transform->pos[0];
    
    REQUIRE(std::abs(actualMovement - expectedMovement) < 0.001f);
}

TEST_CASE("CameraSystem vertical movement (Space/C keys)", "[camera]") {
    arena::ecs::Registry registry;
    arena::ecs::CameraSystem cameraSystem;
    
    // Create a camera entity
    arena::ecs::Entity camera = registry.create();
    registry.add<arena::ecs::Transform>(camera, {{0, 0, 5}, {0, 0, 0}, {1, 1, 1}});
    registry.add<arena::ecs::CameraController>(camera, {5.0f, 0.002f});
    
    auto* transform = registry.get<arena::ecs::Transform>(camera);
    
    // Test Space key (up)
    arena::InputState input = {};
    input.keys[GLFW_KEY_SPACE] = true;
    
    const float dt = 1.0f / 60.0f;
    const int frames = 30;
    
    float initialY = transform->pos[1];
    
    for (int i = 0; i < frames; ++i) {
        cameraSystem.update(dt, input, registry);
    }
    
    // Check that Y position increased (moved up)
    REQUIRE(transform->pos[1] > initialY);
    
    // Test C key (down)
    input.keys[GLFW_KEY_SPACE] = false;
    input.keys[GLFW_KEY_C] = true;
    
    float midY = transform->pos[1];
    
    for (int i = 0; i < frames; ++i) {
        cameraSystem.update(dt, input, registry);
    }
    
    // Check that Y position decreased (moved down)
    REQUIRE(transform->pos[1] < midY);
}
