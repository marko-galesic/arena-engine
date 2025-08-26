#pragma once

// Forward declarations
struct GLFWwindow;
typedef void (*GLFWwindowclosefun)(GLFWwindow*);
typedef void (*GLFWkeyfun)(GLFWwindow*, int, int, int, int);
typedef void (*GLFWcursorposfun)(GLFWwindow*, double, double);
typedef void (*GLFWmousebuttonfun)(GLFWwindow*, int, int, int);

#include <string>

namespace arena::gfx {

class GLContext {
public:
    GLContext();
    ~GLContext();
    
    // Initialize GLFW and create OpenGL 4.5 core profile context
    bool initialize(int width, int height, const std::string& title);
    
    // Check if context is valid
    bool isValid() const { return window_ != nullptr; }
    
    // Get the GLFW window
    GLFWwindow* getWindow() const { return window_; }
    
    // Make context current
    void makeCurrent();
    
    // Swap buffers
    void swapBuffers();
    
    // Poll events
    void pollEvents();
    
    // Check if window should close
    bool shouldClose() const;
    
    // Get framebuffer size
    void getFramebufferSize(int* width, int* height) const;
    
    // Set window close callback
    void setWindowCloseCallback(GLFWwindowclosefun callback);
    
    // Set key callback
    void setKeyCallback(GLFWkeyfun callback);
    
    // Set cursor position callback
    void setCursorPosCallback(GLFWcursorposfun callback);
    
    // Set mouse button callback
    void setMouseButtonCallback(GLFWmousebuttonfun callback);
    
    // Cleanup
    void cleanup();

private:
    GLFWwindow* window_;
    
    // Setup OpenGL state after context creation
    void setupOpenGLState();
};

} // namespace arena::gfx
