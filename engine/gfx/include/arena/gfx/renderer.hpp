#pragma once

#include "arena/gfx/shader.hpp"
#include "arena/gfx/mesh.hpp"
#include "arena/gfx/material.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace arena::gfx {

struct FrameParams { 
    int fbW, fbH; 
    float alpha; 
};

struct DrawItem { 
    Mesh* mesh; 
    glm::mat4 M; 
};

struct FrameState {
    glm::mat4 V, P; 
    std::vector<DrawItem> items;
    glm::vec3 sunDir, sunColor;
};

class Renderer {
public:
    bool init();
    void shutdown();
    void beginFrame(const FrameParams& fp);
    void endFrame();
    
    // Draw list management
    void submit(Mesh* mesh, const glm::mat4& M);
    void flush(const FrameState& frame);
    
    Shader& basic(); // access to a default shader

private:
    Shader basic_;
    std::vector<DrawItem> drawItems_;
    
    // Uniform locations for basic shader
    GLint modelLoc_ = -1;
    GLint viewLoc_ = -1;
    GLint projLoc_ = -1;
    
    void updateUniformLocations();
};

} // namespace arena::gfx
