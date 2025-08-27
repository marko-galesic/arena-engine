#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "arena/ecs/components.hpp"

namespace arena {

// Create a perspective projection matrix
inline glm::mat4 perspective(float fovY, float aspect, float near, float far) {
    return glm::perspective(glm::radians(fovY), aspect, near, far);
}

// Create a look-at view matrix
inline glm::mat4 lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) {
    return glm::lookAt(eye, center, up);
}

// Create a translation matrix
inline glm::mat4 translate(const glm::vec3& translation) {
    return glm::translate(glm::mat4(1.0f), translation);
}

// Create a rotation matrix from yaw, pitch, roll (in radians)
inline glm::mat4 rotateYawPitchRoll(float yaw, float pitch, float roll) {
    glm::mat4 rot = glm::mat4(1.0f);
    rot = glm::rotate(rot, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    rot = glm::rotate(rot, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    rot = glm::rotate(rot, roll, glm::vec3(0.0f, 0.0f, 1.0f));
    return rot;
}

// Create a scale matrix
inline glm::mat4 scale(const glm::vec3& scale) {
    return glm::scale(glm::mat4(1.0f), scale);
}

} // namespace arena
