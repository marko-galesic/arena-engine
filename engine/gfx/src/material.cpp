#include "arena/gfx/material.hpp"
#include <glad/gl.h>

namespace arena::gfx {

Material::Material(Shader* shader) : shader_(shader) {
}

void Material::setShader(Shader* shader) {
    shader_ = shader;
    // Reset uniform locations since shader changed
    colorLoc_ = -1;
    sunDirLoc_ = -1;
    sunColorLoc_ = -1;
}

void Material::setColor(const glm::vec3& color) {
    color_ = color;
}

void Material::setSunDirection(const glm::vec3& direction) {
    sunDir_ = glm::normalize(direction);
}

void Material::setSunColor(const glm::vec3& color) {
    sunColor_ = color;
}

void Material::bind() const {
    if (!shader_) return;
    
    shader_->use();
    updateUniformLocations();
    
    // Set uniforms
    if (colorLoc_ != -1) {
        glUniform3fv(colorLoc_, 1, &color_[0]);
    }
    if (sunDirLoc_ != -1) {
        glUniform3fv(sunDirLoc_, 1, &sunDir_[0]);
    }
    if (sunColorLoc_ != -1) {
        glUniform3fv(sunColorLoc_, 1, &sunColor_[0]);
    }
}

void Material::updateUniformLocations() const {
    if (colorLoc_ == -1) {
        colorLoc_ = shader_->uni("uColor");
    }
    if (sunDirLoc_ == -1) {
        sunDirLoc_ = shader_->uni("uSunDir");
    }
    if (sunColorLoc_ == -1) {
        sunColorLoc_ = shader_->uni("uSunColor");
    }
}

} // namespace arena::gfx

