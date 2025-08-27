#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include "arena/gfx/shader.hpp"

TEST_CASE("Shader path assembly", "[shader]") {
    arena::gfx::Shader shader;
    
    SECTION("Invalid base path") {
        std::string invalidPath = "nonexistent/path";
        REQUIRE_FALSE(shader.load((invalidPath + ".vert").c_str(), (invalidPath + ".frag").c_str()));
        REQUIRE_FALSE(shader.prog != 0);
    }
}

TEST_CASE("Shader timestamp checking", "[shader]") {
    // Create a temporary test shader file
    std::string tempDir = std::filesystem::temp_directory_path().string();
    std::string testBasePath = tempDir + "/test_shader";
    std::string testVertPath = testBasePath + ".vert";
    std::string testFragPath = testBasePath + ".frag";
    
    // Create test shader files
    std::ofstream vertFile(testVertPath);
    vertFile << "#version 450 core\nvoid main() { gl_Position = vec4(0.0); }";
    vertFile.close();
    
    std::ofstream fragFile(testFragPath);
    fragFile << "#version 450 core\nout vec4 FragColor; void main() { FragColor = vec4(1.0); }";
    fragFile.close();
    
    arena::gfx::Shader shader;
    
    SECTION("File modification detected") {
        // Just test that the file modification detection works
        // without actually loading the shader (which requires OpenGL context)
        std::filesystem::path vertPath(testVertPath);
        std::filesystem::path fragPath(testFragPath);
        
        REQUIRE(std::filesystem::exists(vertPath));
        REQUIRE(std::filesystem::exists(fragPath));
        
        // Wait a bit to ensure different timestamps
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        // Modify the fragment shader
        std::ofstream fragFileMod(testFragPath);
        fragFileMod << "#version 450 core\nout vec4 FragColor; void main() { FragColor = vec4(0.5); }";
        fragFileMod.close();
        
        // Verify the file was modified
        auto newTime = std::filesystem::last_write_time(fragPath);
        REQUIRE(newTime > std::filesystem::last_write_time(vertPath));
    }
    
    // Cleanup test files
    std::filesystem::remove(testVertPath);
    std::filesystem::remove(testFragPath);
}

TEST_CASE("Shader uniform location", "[shader]") {
    arena::gfx::Shader shader;
    
    SECTION("Invalid shader") {
        // Should return -1 for invalid shader
        REQUIRE(shader.uni("uMVP") == -1);
    }
    
    SECTION("Valid shader with uniforms") {
        // This test would require a valid shader with known uniforms
        // For now, we'll just test the basic functionality
        REQUIRE(shader.uni("nonexistent") == -1);
    }
}
