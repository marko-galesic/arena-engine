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
    
    // Get uniform locations
    updateUniformLocations();
    
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
    
    // Clear draw list
    drawItems_.clear();
    
    // Set viewport and clear framebuffer
    glViewport(0, 0, fp.fbW, fp.fbH);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    // Frame rendering is complete
}

void Renderer::submit(Mesh* mesh, const glm::mat4& M) {
    if (mesh) {
        drawItems_.push_back({mesh, M});
    }
}

void Renderer::flush(const FrameState& frame) {
    if (drawItems_.empty()) return;
    
    // Use basic shader
    basic_.use();
    
    // Set view and projection matrices
    if (viewLoc_ != -1) {
        glUniformMatrix4fv(viewLoc_, 1, GL_FALSE, &frame.V[0][0]);
    }
    if (projLoc_ != -1) {
        glUniformMatrix4fv(projLoc_, 1, GL_FALSE, &frame.P[0][0]);
    }
    
    // Set sun uniforms
    GLint sunDirLoc = basic_.uni("uSunDir");
    GLint sunColorLoc = basic_.uni("uSunColor");
    if (sunDirLoc != -1) {
        glUniform3fv(sunDirLoc, 1, &frame.sunDir[0]);
    }
    if (sunColorLoc != -1) {
        glUniform3fv(sunColorLoc, 1, &frame.sunColor[0]);
    }
    
    // Render all items
    for (const auto& item : drawItems_) {
        if (!item.mesh) continue;
        
        // Set model matrix
        if (modelLoc_ != -1) {
            glUniformMatrix4fv(modelLoc_, 1, GL_FALSE, &item.M[0][0]);
        }
        
        // Bind mesh and draw
        item.mesh->bind();
        glDrawElements(GL_TRIANGLES, item.mesh->indexCount, GL_UNSIGNED_INT, 0);
    }
}

Shader& Renderer::basic() {
    return basic_;
}

void Renderer::updateUniformLocations() {
    basic_.use();
    modelLoc_ = basic_.uni("uM");
    viewLoc_ = basic_.uni("uV");
    projLoc_ = basic_.uni("uP");
}

} // namespace arena::gfx
