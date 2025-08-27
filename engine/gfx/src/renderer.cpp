#include "arena/gfx/renderer.hpp"
#include <glad/gl.h>
#include <iostream>

namespace arena::gfx {

bool Renderer::init() {
    // Enable sRGB framebuffer and depth testing
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Load the basic shader
    if (!basic_.load("assets/shaders/basic.vert", "assets/shaders/basic.frag")) {
        std::cerr << "ERROR: Failed to load basic shader" << std::endl;
        return false;
    }
    
    std::cout << "[renderer] initialized successfully" << std::endl;
    return true;
}

void Renderer::shutdown() {
    // Shader cleanup is handled automatically by the Shader struct
    std::cout << "[renderer] shutdown complete" << std::endl;
}

void Renderer::beginFrame(const FrameParams& fp) {
    // Check for shader hot-reload
    basic_.reloadIfChanged();
    
    // Set viewport and clear framebuffer
    glViewport(0, 0, fp.fbW, fp.fbH);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    // Frame rendering is complete
}

Shader& Renderer::basic() {
    return basic_;
}

} // namespace arena::gfx
