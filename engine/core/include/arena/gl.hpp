#pragma once
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#include "arena/gl_canary.hpp"  // see below
#include <glad/gl.h>            // GLAD2 header
#include <GLFW/glfw3.h>

inline bool arena_load_gl() {
    return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // GLAD2 init
}
