#include <catch2/catch_test_macros.hpp>
#include "arena/gfx/renderer.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

TEST_CASE("Shader hot-reload functionality", "[hot_reload]") {
    // Initialize GLFW
    REQUIRE(glfwInit() == GLFW_TRUE);
    
    // Create a window (hidden) for OpenGL context
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    REQUIRE(window != nullptr);
    
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    REQUIRE(arena_load_gl());
    
    SECTION("Hot-reload detection") {
        arena::gfx::Renderer renderer;
        REQUIRE(renderer.init() == true);
        
        // Get initial shader program ID
        GLuint initialProgId = renderer.basic().prog;
        REQUIRE(initialProgId != 0);
        
        // Simulate frame rendering (this should check for hot-reload)
        arena::gfx::FrameParams params{800, 600, 1.0f};
        renderer.beginFrame(params);
        
        // The shader should still be the same since no files were modified
        REQUIRE(renderer.basic().prog == initialProgId);
        
        renderer.shutdown();
    }
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}
