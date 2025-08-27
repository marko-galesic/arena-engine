#include <catch2/catch_test_macros.hpp>
#include "arena/gfx/renderer.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

TEST_CASE("Renderer initialization and basic functionality", "[renderer]") {
    // Check if we're in a CI environment
    const char* ci_env = std::getenv("CI");
    const char* github_actions = std::getenv("GITHUB_ACTIONS");
    const char* azure_pipelines = std::getenv("AZURE_PIPELINES");
    
    bool is_ci = (ci_env != nullptr && std::strcmp(ci_env, "true") == 0) ||
                 (github_actions != nullptr && std::strcmp(github_actions, "true") == 0) ||
                 (azure_pipelines != nullptr && std::strcmp(azure_pipelines, "true") == 0);

    // Initialize GLFW
    REQUIRE(glfwInit() == GLFW_TRUE);
    
    // Create a window (hidden) for OpenGL context
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    
    if (!window && is_ci) {
        // In CI environments, GLFW failing to create a window is expected
        std::cout << "GLFW: Failed to create window (expected in CI environment)" << std::endl;
        std::cout << "GLFW: Skipping renderer test - this is normal in CI" << std::endl;
        glfwTerminate();
        return; // Skip the test since this is expected behavior
    }
    
    REQUIRE(window != nullptr);
    
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    REQUIRE(arena_load_gl());
    
    SECTION("Renderer initialization") {
        arena::gfx::Renderer renderer;
        REQUIRE(renderer.init() == true);
        
        // Test that basic shader is accessible
        REQUIRE(renderer.basic().prog != 0);
        
        renderer.shutdown();
    }
    
    SECTION("Frame rendering") {
        arena::gfx::Renderer renderer;
        REQUIRE(renderer.init() == true);
        
        arena::gfx::FrameParams params{800, 600, 1.0f};
        
        // Test frame begin/end
        renderer.beginFrame(params);
        renderer.endFrame();
        
        // Verify OpenGL state
        GLboolean sRGBEnabled;
        glGetBooleanv(GL_FRAMEBUFFER_SRGB, &sRGBEnabled);
        REQUIRE(sRGBEnabled == GL_TRUE);
        
        GLboolean depthTestEnabled;
        glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
        REQUIRE(depthTestEnabled == GL_TRUE);
        
        renderer.shutdown();
    }
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}
