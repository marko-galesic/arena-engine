#pragma once

#include "arena/gl.hpp"
#include <glm/glm.hpp>

namespace arena::gfx {

struct Mesh {
    GLuint vao = 0, vbo = 0, ibo = 0;
    GLsizei indexCount = 0;
    
    // Create a grid mesh with thin quads forming lines
    static Mesh makeGrid(int half = 16, float cell = 1.0f);
    
    // Create a cube mesh with 36 indices
    static Mesh makeCube(float s = 1.0f);
    
    // Create a simple floor plane
    static Mesh makeFloor(float size = 20.0f);
    
    // Cleanup OpenGL resources
    void destroy();
    
    // Bind the mesh for rendering
    void bind() const;
};

} // namespace arena::gfx
