#include <catch2/catch_test_macros.hpp>
#include "arena/gfx/renderer.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cstring>

TEST_CASE("Shader hot-reload demonstration", "[hot_reload_demo]") {
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
        std::cout << "GLFW: Skipping shader hot-reload demo test - this is normal in CI" << std::endl;
        glfwTerminate();
        return; // Skip the test since this is expected behavior
    }
    
    REQUIRE(window != nullptr);
    
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    REQUIRE(arena_load_gl());
    
    SECTION("Hot-reload demonstration") {
        arena::gfx::Renderer renderer;
        REQUIRE(renderer.init() == true);
        
        // Get initial shader program ID
        GLuint initialProgId = renderer.basic().prog;
        REQUIRE(initialProgId != 0);
        
        // Create a temporary test shader file to demonstrate hot-reload
        std::string tempDir = std::filesystem::temp_directory_path().string();
        std::string testVertPath = tempDir + "/test_basic.vert";
        std::string testFragPath = tempDir + "/test_basic.frag";
        
        // Create initial test shader files
        {
            std::ofstream vertFile(testVertPath);
            vertFile << "#version 450 core\nlayout(location=0) in vec3 aPos;\nvoid main(){ gl_Position = vec4(aPos,1.0); }";
            vertFile.close();
            
            std::ofstream fragFile(testFragPath);
            fragFile << "#version 450 core\nout vec4 FragColor;\nvoid main(){ FragColor = vec4(1.0,0.0,0.0,1.0); }";
            fragFile.close();
        }
        
        // Create a test shader and load it
        arena::gfx::Shader testShader;
        REQUIRE(testShader.load(testVertPath.c_str(), testFragPath.c_str()));
        GLuint testProgId = testShader.prog;
        REQUIRE(testProgId != 0);
        
        // Modify the fragment shader file
        {
            std::ofstream fragFile(testFragPath);
            fragFile << "#version 450 core\nout vec4 FragColor;\nvoid main(){ FragColor = vec4(0.0,1.0,0.0,1.0); }";
            fragFile.close();
        }
        
        // Wait a bit to ensure file timestamp changes and flush file system
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // Force file system sync by touching the file again
        {
            std::ofstream touchFile(testFragPath, std::ios::app);
            touchFile << "\n// Force timestamp update";
            touchFile.close();
        }
        
        // Wait a bit more for the file system to settle
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        // Check for hot-reload
        bool reloaded = testShader.reloadIfChanged();
        REQUIRE(reloaded == true);
        
        // The program ID should have changed
        REQUIRE(testShader.prog != testProgId);
        REQUIRE(testShader.prog != 0);
        
        // Cleanup test files
        std::filesystem::remove(testVertPath);
        std::filesystem::remove(testFragPath);
        
        renderer.shutdown();
    }
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}
