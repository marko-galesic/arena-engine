#include "arena/gl.hpp"
#include <cstdio>
#include <cstdlib>

int main() {
    glfwSetErrorCallback([](int c,const char* m){ fprintf(stderr,"GLFW[%d]: %s\n",c,m); });
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(64, 64, "glad-init", nullptr, nullptr);
    if (!win) return 2;

    glfwMakeContextCurrent(win);
    if (!arena_load_gl()) return 3;

    if (!glCreateShader || !glBufferData || !glDrawArrays) return 4;

    GLint maj=0,min=0,mask=0;
    glGetIntegerv(GL_MAJOR_VERSION,&maj);
    glGetIntegerv(GL_MINOR_VERSION,&min);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK,&mask);
    if (maj < 3 || (maj==3 && min<3) || !(mask & GL_CONTEXT_CORE_PROFILE_BIT)) return 5;

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
