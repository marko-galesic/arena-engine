#include "arena/gl.hpp"
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

    GLFWwindow* win = glfwCreateWindow(64, 64, "glad-init", nullptr, nullptr);
    if (!win) {
        if (is_ci) {
            // In CI environments, GLFW failing to create a window is expected
            fprintf(stderr, "GLFW: Failed to create window (expected in CI environment)\n");
            fprintf(stderr, "GLFW: Skipping OpenGL context test - this is normal in CI\n");
            glfwTerminate();
            return 0; // Return success since this is expected behavior
        } else {
            // In local environments, this should be a failure
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

    if (!glCreateShader || !glBufferData || !glDrawArrays) {
        fprintf(stderr, "GLAD: Required OpenGL functions not loaded\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return 4;
    }

    GLint maj=0,min=0,mask=0;
    glGetIntegerv(GL_MAJOR_VERSION,&maj);
    glGetIntegerv(GL_MINOR_VERSION,&min);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK,&mask);
    if (maj < 4 || (maj==4 && min<5) || !(mask & GL_CONTEXT_CORE_PROFILE_BIT)) {
        fprintf(stderr, "GLAD: OpenGL version/profile requirements not met\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return 5;
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
