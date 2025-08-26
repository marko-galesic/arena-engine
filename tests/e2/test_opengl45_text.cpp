#include "arena/gl.hpp"
#include "arena/text.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main() {
    // Check if we're in a CI environment
    const char* ci_env = std::getenv("CI");
    const char* github_actions = std::getenv("GITHUB_ACTIONS");
    const char* azure_pipelines = std::getenv("AZURE_PIPELINES");
    
    bool is_ci = (ci_env != nullptr && std::strcmp(ci_env, "true") == 0) ||
                 (github_actions != nullptr && std::strcmp(github_actions, "true") == 0) ||
                 (azure_pipelines != nullptr && std::strcmp(azure_pipelines, "true") == 0);

    glfwSetErrorCallback([](int c,const char* m){ fprintf(stderr,"GLFW[%d]: %s\n",c,m); });
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* win = glfwCreateWindow(64, 64, "opengl45-text-test", nullptr, nullptr);
    if (!win) {
        if (is_ci) {
            fprintf(stderr, "GLFW: Failed to create window (expected in CI environment)\n");
            fprintf(stderr, "GLFW: Skipping OpenGL 4.5 text test - this is normal in CI\n");
            glfwTerminate();
            return 0; // Return success since this is expected behavior
        } else {
            fprintf(stderr, "GLFW: Failed to create window\n");
            glfwTerminate();
            return 2;
        }
    }

    glfwMakeContextCurrent(win);
    if (!arena_load_gl()) {
        fprintf(stderr, "GLAD: Failed to load OpenGL functions\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return 3;
    }

    // Verify OpenGL 4.5 is available
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 4 || (major == 4 && minor < 5)) {
        fprintf(stderr, "GLAD: OpenGL 4.5 required for text rendering, but got version %d.%d\n", major, minor);
        glfwDestroyWindow(win);
        glfwTerminate();
        return 4;
    }

    // Test OpenGL 4.5 specific functions
    if (!glCreateVertexArrays || !glNamedBufferData || !glProgramUniform4f) {
        fprintf(stderr, "GLAD: Required OpenGL 4.5 functions not loaded\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return 5;
    }

    fprintf(stderr, "OpenGL %d.%d context created successfully\n", major, minor);
    fprintf(stderr, "Vendor: %s\n", glGetString(GL_VENDOR));
    fprintf(stderr, "Renderer: %s\n", glGetString(GL_RENDERER));

    // Test text HUD initialization
    try {
        arena::hud::TextHud_Init();
        fprintf(stderr, "Text HUD initialization successful\n");
        
        // Test basic text rendering setup
        arena::hud::TextHud_BeginFrame(800, 600);
        fprintf(stderr, "Text HUD frame setup successful\n");
        
        // Cleanup
        arena::hud::TextHud_Shutdown();
        fprintf(stderr, "Text HUD cleanup successful\n");
        
    } catch (const std::exception& e) {
        fprintf(stderr, "Text HUD test failed with exception: %s\n", e.what());
        glfwDestroyWindow(win);
        glfwTerminate();
        return 6;
    } catch (...) {
        fprintf(stderr, "Text HUD test failed with unknown exception\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return 7;
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    fprintf(stderr, "OpenGL 4.5 text rendering test completed successfully\n");
    return 0;
}
