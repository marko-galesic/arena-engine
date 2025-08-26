#include "arena/gfx/gl_context.hpp"
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

// GLAD initialization function
inline bool arena_load_gl() {
    return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

namespace arena::gfx {

GLContext::GLContext() : window_(nullptr) {
}

GLContext::~GLContext() {
    cleanup();
}

bool GLContext::initialize(int width, int height, const std::string& title) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Set OpenGL 4.5 context hints for modern features
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    
    // Create window
    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        std::cerr << "ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    // Make context current
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    
    // Initialize GLAD2 after creating the context
    if (!arena_load_gl()) { 
        std::cerr << "ERROR: gladLoadGL failed" << std::endl;
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }
    
    // Verify OpenGL 4.5 is available
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 4 || (major == 4 && minor < 5)) {
        std::cerr << "ERROR: OpenGL 4.5 required, but got version " << major << "." << minor << std::endl;
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }
    
    std::cout << "OpenGL " << major << "." << minor << " context created successfully" << std::endl;
    
    // Quick runtime sanity check
    std::cout << "GL: " << (const char*)glGetString(GL_VENDOR) 
              << " | " << (const char*)glGetString(GL_RENDERER)
              << " | " << (const char*)glGetString(GL_VERSION) << std::endl;
    
    if (!glCreateShader || !glBufferData || !glDrawArrays) {
        std::cerr << "ERROR: GL function pointers are null (likely wrong GLAD combo)" << std::endl;
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }
    
    // Setup OpenGL state
    setupOpenGLState();
    
    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error after initialization: " << err << std::endl;
    }
    
    std::cout << "GLFW window initialized successfully" << std::endl;
    return true;
}

void GLContext::setupOpenGLState() {
    // Enable sRGB framebuffer (so writes expect linear, monitor is sRGB)
    glEnable(GL_FRAMEBUFFER_SRGB);
    
    // Set clear color
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable back face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLContext::makeCurrent() {
    if (window_) {
        glfwMakeContextCurrent(window_);
    }
}

void GLContext::swapBuffers() {
    if (window_) {
        glfwSwapBuffers(window_);
    }
}

void GLContext::pollEvents() {
    glfwPollEvents();
}

bool GLContext::shouldClose() const {
    return window_ ? glfwWindowShouldClose(window_) : true;
}

void GLContext::getFramebufferSize(int* width, int* height) const {
    if (window_) {
        glfwGetFramebufferSize(window_, width, height);
    } else {
        *width = 0;
        *height = 0;
    }
}

void GLContext::setWindowCloseCallback(GLFWwindowclosefun callback) {
    if (window_) {
        glfwSetWindowCloseCallback(window_, callback);
    }
}

void GLContext::setKeyCallback(GLFWkeyfun callback) {
    if (window_) {
        glfwSetKeyCallback(window_, callback);
    }
}

void GLContext::setCursorPosCallback(GLFWcursorposfun callback) {
    if (window_) {
        glfwSetCursorPosCallback(window_, callback);
    }
}

void GLContext::setMouseButtonCallback(GLFWmousebuttonfun callback) {
    if (window_) {
        glfwSetMouseButtonCallback(window_, callback);
    }
}

void GLContext::cleanup() {
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

} // namespace arena::gfx
