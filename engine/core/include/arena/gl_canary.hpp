#pragma once
#if defined(GLAD_GLAD_H_)
# error "GLAD1 header (<glad/glad.h>) detected. This project uses GLAD2 (<glad/gl.h>) only."
#endif
#if defined(__gl_h_) || defined(__GL_H__) || defined(GL_GL_H) || defined(__WGL_H__)
# error "System OpenGL headers detected. Use arena/gl.hpp (GLAD2 + GLFW) only."
#endif
