#include <catch2/catch_test_macros.hpp>
#include "arena/gfx/renderer.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

TEST_CASE("Shader hot-reload functionality", "[hot_reload]") {
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
        std::cout << "GLFW: Skipping shader hot-reload test - this is normal in CI" << std::endl;
        glfwTerminate();
        return; // Skip the test since this is expected behavior
    }
    
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
