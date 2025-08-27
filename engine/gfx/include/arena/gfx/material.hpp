#pragma once

#include "arena/gfx/shader.hpp"
#include <glm/glm.hpp>

namespace arena::gfx {

class Material {
public:
    Material(Shader* shader = nullptr);
    
    // Set the shader for this material
    void setShader(Shader* shader);
    
    // Set material properties
    void setColor(const glm::vec3& color);
    void setSunDirection(const glm::vec3& direction);
    void setSunColor(const glm::vec3& color);
    
    // Bind the material (sets uniforms)
    void bind() const;
    
    // Get the shader
    Shader* getShader() const { return shader_; }

private:
    Shader* shader_;
    
    // Uniform locations
    mutable GLint colorLoc_ = -1;
    mutable GLint sunDirLoc_ = -1;
    mutable GLint sunColorLoc_ = -1;
    
    // Material properties
    glm::vec3 color_ = {0.75f, 0.75f, 0.78f};
    glm::vec3 sunDir_ = {-0.3f, -1.0f, -0.2f};
    glm::vec3 sunColor_ = {1.0f, 0.95f, 0.9f};
    
    // Get uniform locations (cached)
    void updateUniformLocations() const;
};

} // namespace arena::gfx

