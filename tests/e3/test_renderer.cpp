#include <catch2/catch_test_macros.hpp>
#include "arena/gfx/renderer.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

TEST_CASE("Renderer initialization and basic functionality", "[renderer]") {
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
