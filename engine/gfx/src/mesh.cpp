#include "arena/gfx/mesh.hpp"
#include <glad/gl.h>
#include <vector>
#include <array>
#include <cmath>

namespace arena::gfx {

Mesh Mesh::makeGrid(int half, float cell) {
    Mesh mesh;

    // We draw the grid as very thin quads (two triangles) lying on Y=0.
    // Vertex format: position (xyz) + color (rgb) = 6 floats.
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    const float extent      = half * cell;   // half-size of the grid in world units
    const float step        = 0.25f;         // spacing between lines
    const int   gridCount   = static_cast<int>(std::floor(extent / step));
    const float halfWidth   = cell * 0.005f; // ~0.01 wide lines (feel free to tweak)

    // Colors: major lines (every 1.0 unit) are darker; minor lines lighter.
    auto lineColor = [](bool major) -> std::array<float,3> {
        // On a dark background (0.1), darker majors read as ~0.25 grey,
        // minor lines are lighter (~0.45 grey).
        return major ? std::array<float,3>{0.25f, 0.25f, 0.25f}
                     : std::array<float,3>{0.45f, 0.45f, 0.45f};
    };

    auto push_quad = [&](float x0, float z0, float x1, float z1, const std::array<float,3>& c) {
        // CCW winding, top-facing
        unsigned int base = static_cast<unsigned int>(vertices.size() / 6);
        vertices.insert(vertices.end(), { x0, 0.0f, z0,  c[0], c[1], c[2] });
        vertices.insert(vertices.end(), { x1, 0.0f, z0,  c[0], c[1], c[2] });
        vertices.insert(vertices.end(), { x1, 0.0f, z1,  c[0], c[1], c[2] });
        vertices.insert(vertices.end(), { x0, 0.0f, z1,  c[0], c[1], c[2] });
        indices.insert(indices.end(), { base, base+1, base+2,  base, base+2, base+3 });
    };

    // Vertical lines (vary X, span Z)
    for (int i = -gridCount; i <= gridCount; ++i) {
        const float x = i * step;
        const bool  major = (i % 4 == 0);      // every 1.0 unit
        const auto  c = lineColor(major);
        push_quad(x - halfWidth, -extent, x + halfWidth, +extent, c);
    }

    // Horizontal lines (vary Z, span X)
    for (int i = -gridCount; i <= gridCount; ++i) {
        const float z = i * step;
        const bool  major = (i % 4 == 0);      // every 1.0 unit
        const auto  c = lineColor(major);
        push_quad(-extent, z - halfWidth, +extent, z + halfWidth, c);
    }

    // ---- GPU buffers / VAO ----
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position (location = 0) : vec3, stride = 6 floats
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // Color (location = 1) : vec3, offset = 3 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    mesh.indexCount = static_cast<GLsizei>(indices.size());

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return mesh;
}

Mesh Mesh::makeCube(float s) {
    Mesh mesh;
    
    // Cube vertices with position, normal, and UV
    // Each face has 4 vertices, 6 indices (2 triangles)
    std::vector<float> vertices = {
        // Front face
        -s, -s,  s,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  // 0
         s, -s,  s,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,  // 1
         s,  s,  s,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,  // 2
        -s,  s,  s,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,  // 3
        
        // Back face
        -s, -s, -s,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,  // 4
        -s,  s, -s,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,  // 5
         s,  s, -s,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,  // 6
         s, -s, -s,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,  // 7
        
        // Left face
        -s, -s, -s, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // 8
        -s, -s,  s, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // 9
        -s,  s,  s, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // 10
        -s,  s, -s, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // 11
        
        // Right face
         s, -s, -s,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  // 12
         s,  s, -s,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  // 13
         s,  s,  s,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  // 14
         s, -s,  s,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  // 15
        
        // Bottom face
        -s, -s, -s,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,  // 16
         s, -s, -s,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  // 17
         s, -s,  s,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  // 18
        -s, -s,  s,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  // 19
        
        // Top face
        -s,  s, -s,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  // 20
        -s,  s,  s,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,  // 21
         s,  s,  s,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  // 22
         s,  s, -s,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f   // 23
    };
    
    // Indices for all faces (36 total)
    std::vector<unsigned int> indices = {
        0,  1,  2,   0,  2,  3,   // Front
        4,  5,  6,   4,  6,  7,   // Back
        8,  9,  10,  8,  10, 11,  // Left
        12, 13, 14,  12, 14, 15,  // Right
        16, 17, 18,  16, 18, 19,  // Bottom
        20, 21, 22,  20, 22, 23   // Top
    };
    
    // Create and bind VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    
    // Create and bind VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Set vertex attributes
    // Position (3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    
    // Normal (3 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    
    // UV (2 floats)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    
    // Create and bind IBO
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    mesh.indexCount = static_cast<GLsizei>(indices.size());
    
    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    return mesh;
}

Mesh Mesh::makeColoredCube(float s, float r, float g, float b) {
    Mesh mesh;
    
    // Cube vertices with position and color (6 floats per vertex)
    // Each face has 4 vertices, 6 indices (2 triangles)
    std::vector<float> vertices = {
        // Front face
        -s, -s,  s,  r, g, b,  // 0
         s, -s,  s,  r, g, b,  // 1
         s,  s,  s,  r, g, b,  // 2
        -s,  s,  s,  r, g, b,  // 3
        
        // Back face
        -s, -s, -s,  r, g, b,  // 4
        -s,  s, -s,  r, g, b,  // 5
         s,  s, -s,  r, g, b,  // 6
         s, -s, -s,  r, g, b,  // 7
        
        // Left face
        -s, -s, -s,  r, g, b,  // 8
        -s, -s,  s,  r, g, b,  // 9
        -s,  s,  s,  r, g, b,  // 10
        -s,  s, -s,  r, g, b,  // 11
        
        // Right face
         s, -s, -s,  r, g, b,  // 12
         s,  s, -s,  r, g, b,  // 13
         s,  s,  s,  r, g, b,  // 14
         s, -s,  s,  r, g, b,  // 15
        
        // Bottom face
        -s, -s, -s,  r, g, b,  // 16
         s, -s, -s,  r, g, b,  // 17
         s, -s,  s,  r, g, b,  // 18
        -s, -s,  s,  r, g, b,  // 19
        
        // Top face
        -s,  s, -s,  r, g, b,  // 20
        -s,  s,  s,  r, g, b,  // 21
         s,  s,  s,  r, g, b,  // 22
         s,  s, -s,  r, g, b   // 23
    };
    
    // Indices for all faces (36 total)
    std::vector<unsigned int> indices = {
        0,  1,  2,   0,  2,  3,   // Front
        4,  5,  6,   4,  6,  7,   // Back
        8,  9,  10,  8,  10, 11,  // Left
        12, 13, 14,  12, 14, 15,  // Right
        16, 17, 18,  16, 18, 19,  // Bottom
        20, 21, 22,  20, 22, 23   // Top
    };
    
    // Create and bind VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    
    // Create and bind VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Set vertex attributes
    // Position (3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    
    // Color (3 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    // Create and bind IBO
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    mesh.indexCount = static_cast<GLsizei>(indices.size());
    
    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    return mesh;
}

Mesh Mesh::makeFloor(float size) {
    Mesh mesh;
    
    // Create a simple floor plane
    // 4 vertices forming a quad, 6 indices (2 triangles)
    std::vector<float> vertices = {
        // Position (3 floats), Normal (3 floats), UV (2 floats)
        -size, 0.0f, -size,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  // Bottom-left
         size, 0.0f, -size,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  // Bottom-right
         size, 0.0f,  size,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,  // Top-right
        -size, 0.0f,  size,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f   // Top-left
    };
    
    // Indices for the floor quad (2 triangles)
    std::vector<unsigned int> indices = {
        0, 1, 2,  // First triangle
        0, 2, 3   // Second triangle
    };
    
    // Create and bind VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    
    // Create and bind VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Set vertex attributes
    // Position (3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    
    // Normal (3 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    
    // UV (2 floats)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    
    // Create and bind IBO
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    mesh.indexCount = static_cast<GLsizei>(indices.size());
    
    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    return mesh;
}

void Mesh::destroy() {
    if (ibo) {
        glDeleteBuffers(1, &ibo);
        ibo = 0;
    }
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    indexCount = 0;
}

void Mesh::bind() const {
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

Mesh Mesh::makeCoordinateAxes(float length, float thickness) {
    Mesh mesh;
    
    // Create coordinate axes as thick lines
    // X-axis: Red, Y-axis: Green, Z-axis: Blue
    // Each axis is a thin box (6 faces, 24 vertices, 36 indices)
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    const float halfThickness = thickness * 0.5f;
    
    // Helper function to add a box for an axis
    auto addAxisBox = [&](float startX, float startY, float startZ, 
                          float endX, float endY, float endZ,
                          float r, float g, float b) {
        // Calculate the direction vector
        float dx = endX - startX;
        float dy = endY - startY;
        float dz = endZ - startZ;
        
        // Create a thin box along the axis direction
        // We'll create a box that's thin in the perpendicular directions
        
        // For X-axis: thin in Y and Z
        // For Y-axis: thin in X and Z  
        // For Z-axis: thin in X and Y
        
        // Calculate perpendicular vectors for the thin dimensions
        float perp1X, perp1Y, perp1Z, perp2X, perp2Y, perp2Z;
        
        if (std::abs(dx) > std::abs(dy) && std::abs(dx) > std::abs(dz)) {
            // X-axis dominant - thin in Y and Z
            perp1X = 0.0f; perp1Y = 1.0f; perp1Z = 0.0f;
            perp2X = 0.0f; perp2Y = 0.0f; perp2Z = 1.0f;
        } else if (std::abs(dy) > std::abs(dz)) {
            // Y-axis dominant - thin in X and Z
            perp1X = 1.0f; perp1Y = 0.0f; perp1Z = 0.0f;
            perp2X = 0.0f; perp2Y = 0.0f; perp2Z = 1.0f;
        } else {
            // Z-axis dominant - thin in X and Y
            perp1X = 1.0f; perp1Y = 0.0f; perp1Z = 0.0f;
            perp2X = 0.0f; perp2Y = 1.0f; perp2Z = 0.0f;
        }
        
        // Create 8 vertices for the box
        unsigned int base = static_cast<unsigned int>(vertices.size() / 6);
        
        // Bottom face vertices
        vertices.insert(vertices.end(), {
            startX - perp1X * halfThickness - perp2X * halfThickness, 
            startY - perp1Y * halfThickness - perp2Y * halfThickness, 
            startZ - perp1Z * halfThickness - perp2Z * halfThickness, r, g, b,
            
            startX + perp1X * halfThickness - perp2X * halfThickness, 
            startY + perp1Y * halfThickness - perp2Y * halfThickness, 
            startZ + perp1Z * halfThickness - perp2Z * halfThickness, r, g, b,
            
            startX + perp1X * halfThickness + perp2X * halfThickness, 
            startY + perp1Y * halfThickness + perp2Y * halfThickness, 
            startZ + perp1Z * halfThickness + perp2Z * halfThickness, r, g, b,
            
            startX - perp1X * halfThickness + perp2X * halfThickness, 
            startY - perp1Y * halfThickness + perp2Y * halfThickness, 
            startZ - perp1Z * halfThickness + perp2Z * halfThickness, r, g, b
        });
        
        // Top face vertices (offset by the axis direction)
        vertices.insert(vertices.end(), {
            endX - perp1X * halfThickness - perp2X * halfThickness, 
            endY - perp1Y * halfThickness - perp2Y * halfThickness, 
            endZ - perp1Z * halfThickness - perp2Z * halfThickness, r, g, b,
            
            endX + perp1X * halfThickness - perp2X * halfThickness, 
            endY + perp1Y * halfThickness - perp2Y * halfThickness, 
            endZ + perp1Z * halfThickness - perp2Z * halfThickness, r, g, b,
            
            endX + perp1X * halfThickness + perp2X * halfThickness, 
            endY + perp1Y * halfThickness + perp2Y * halfThickness, 
            endZ + perp1Z * halfThickness + perp2Z * halfThickness, r, g, b,
            
            endX - perp1X * halfThickness + perp2X * halfThickness, 
            endY - perp1Y * halfThickness + perp2Y * halfThickness, 
            endZ - perp1Z * halfThickness + perp2Z * halfThickness, r, g, b
        });
        
        // Indices for the box (6 faces, 2 triangles each = 12 triangles = 36 indices)
        // Bottom face
        indices.insert(indices.end(), { base+0, base+1, base+2, base+0, base+2, base+3 });
        // Top face  
        indices.insert(indices.end(), { base+4, base+6, base+5, base+4, base+7, base+6 });
        // Side faces
        indices.insert(indices.end(), { base+0, base+4, base+5, base+0, base+5, base+1 });
        indices.insert(indices.end(), { base+1, base+5, base+6, base+1, base+6, base+2 });
        indices.insert(indices.end(), { base+2, base+6, base+7, base+2, base+7, base+3 });
        indices.insert(indices.end(), { base+3, base+7, base+4, base+3, base+4, base+0 });
    };
    
    // Add the three axes with different thicknesses
    // X-axis (red) - thicker
    addAxisBox(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    
    // Y-axis (green) - normal thickness
    addAxisBox(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f);
    
    // Z-axis (blue) - thicker
    addAxisBox(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f);
    
    // Create and bind VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    
    // Create and bind VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Set vertex attributes
    // Position (3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    
    // Color (3 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    // Create and bind IBO
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    mesh.indexCount = static_cast<GLsizei>(indices.size());
    
    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    return mesh;
}

} // namespace arena::gfx
