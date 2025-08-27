#include <catch2/catch_test_macros.hpp>
#include "arena/input.hpp"
#include <GLFW/glfw3.h>

// Prevent GLFW from pulling in legacy OpenGL headers
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

TEST_CASE("InputState initialization", "[input]") {
    arena::InputState state = {};
    
    // Check that all keys are initially false
    for (int i = 0; i < 512; i++) {
        REQUIRE(state.keys[i] == false);
    }
    
    // Check that mouse deltas are initially 0
    REQUIRE(state.mouseDx == 0.0);
    REQUIRE(state.mouseDy == 0.0);
    
    // Check that all mouse buttons are initially false
    for (int i = 0; i < 8; i++) {
        REQUIRE(state.mouseButtons[i] == false);
    }
}

TEST_CASE("Key handling", "[input]") {
    arena::InputState state = {};
    
    // Test W key press (GLFW_KEY_W = 87)
    arena::handleKey(state, GLFW_KEY_W, GLFW_PRESS);
    REQUIRE(state.keys[GLFW_KEY_W] == true);
    
    // Test W key release
    arena::handleKey(state, GLFW_KEY_W, GLFW_RELEASE);
    REQUIRE(state.keys[GLFW_KEY_W] == false);
    
    // Test W key repeat (should keep key pressed for FPS-style movement)
    arena::handleKey(state, GLFW_KEY_W, GLFW_REPEAT);
    REQUIRE(state.keys[GLFW_KEY_W] == true);
    
    // Test other keys
    arena::handleKey(state, GLFW_KEY_A, GLFW_PRESS);
    REQUIRE(state.keys[GLFW_KEY_A] == true);
    
    arena::handleKey(state, GLFW_KEY_SPACE, GLFW_PRESS);
    REQUIRE(state.keys[GLFW_KEY_SPACE] == true);
}

TEST_CASE("Mouse button handling", "[input]") {
    arena::InputState state = {};
    
    // Test left mouse button press (GLFW_MOUSE_BUTTON_LEFT = 0)
    arena::handleMouseButton(state, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
    REQUIRE(state.mouseButtons[GLFW_MOUSE_BUTTON_LEFT] == true);
    
    // Test left mouse button release
    arena::handleMouseButton(state, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
    REQUIRE(state.mouseButtons[GLFW_MOUSE_BUTTON_LEFT] == false);
    
    // Test right mouse button
    arena::handleMouseButton(state, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
    REQUIRE(state.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT] == true);
}

TEST_CASE("Mouse movement with dead-zone", "[input]") {
    arena::InputState state = {};
    
    // Test movement above dead-zone (0.001)
    arena::handleMouseMove(state, 0.002, 0.003);
    REQUIRE(state.mouseDx == 0.002);
    REQUIRE(state.mouseDy == 0.003);
    
    // Test movement below dead-zone (should be clamped to 0)
    arena::handleMouseMove(state, 0.0005, 0.0008);
    REQUIRE(state.mouseDx == 0.0);
    REQUIRE(state.mouseDy == 0.0);
    
    // Test movement exactly at dead-zone boundary
    arena::handleMouseMove(state, 0.001, 0.001);
    REQUIRE(state.mouseDx == 0.001);
    REQUIRE(state.mouseDy == 0.001);
    
    // Test negative movement above dead-zone
    arena::handleMouseMove(state, -0.002, -0.003);
    REQUIRE(state.mouseDx == -0.002);
    REQUIRE(state.mouseDy == -0.003);
    
    // Test negative movement below dead-zone
    arena::handleMouseMove(state, -0.0005, -0.0008);
    REQUIRE(state.mouseDx == 0.0);
    REQUIRE(state.mouseDy == 0.0);
}

TEST_CASE("Frame reset functionality", "[input]") {
    arena::InputState state = {};
    
    // Set some mouse movement
    arena::handleMouseMove(state, 0.5, 0.3);
    REQUIRE(state.mouseDx == 0.5);
    REQUIRE(state.mouseDy == 0.3);
    
    // Begin new frame - should clear deltas
    arena::beginFrame(state);
    REQUIRE(state.mouseDx == 0.0);
    REQUIRE(state.mouseDy == 0.0);
    
    // Keys and mouse buttons should remain unchanged
    arena::handleKey(state, GLFW_KEY_W, GLFW_PRESS);
    arena::handleMouseButton(state, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
    
    arena::beginFrame(state);
    REQUIRE(state.keys[GLFW_KEY_W] == true);
    REQUIRE(state.mouseButtons[GLFW_MOUSE_BUTTON_LEFT] == true);
    REQUIRE(state.mouseDx == 0.0);
    REQUIRE(state.mouseDy == 0.0);
}

TEST_CASE("Boundary conditions", "[input]") {
    arena::InputState state = {};
    
    // Test out-of-bounds key access
    arena::handleKey(state, -1, GLFW_PRESS);
    arena::handleKey(state, 512, GLFW_PRESS);
    arena::handleKey(state, 1000, GLFW_PRESS);
    // Should not crash and should not affect valid keys
    
    // Test out-of-bounds mouse button access
    arena::handleMouseButton(state, -1, GLFW_PRESS);
    arena::handleMouseButton(state, 8, GLFW_PRESS);
    arena::handleMouseButton(state, 100, GLFW_PRESS);
    // Should not crash and should not affect valid buttons
    
    // Test extreme mouse movement values
    arena::handleMouseMove(state, 1000000.0, -1000000.0);
    REQUIRE(state.mouseDx == 1000000.0);
    REQUIRE(state.mouseDy == -1000000.0);
}
