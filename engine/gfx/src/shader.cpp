#include "arena/gfx/shader.hpp"
#include <glad/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace arena::gfx {

Shader::Shader() : programId_(0) {
}

Shader::~Shader() {
    cleanup();
}

bool Shader::load(const std::string& basePath) {
    // Construct paths for vertex and fragment shaders
    vertexPath_ = basePath + ".vert";
    fragmentPath_ = basePath + ".frag";
    
    // Check if files exist
    if (!std::filesystem::exists(vertexPath_) || !std::filesystem::exists(fragmentPath_)) {
        std::cerr << "ERROR: Shader files not found: " << vertexPath_ << " or " << fragmentPath_ << std::endl;
        return false;
    }
    
    // Get initial file timestamps
    vertexLastWrite_ = getFileLastWriteTime(vertexPath_);
    fragmentLastWrite_ = getFileLastWriteTime(fragmentPath_);
    
    // Load and compile shaders
    return compileAndLink();
}

bool Shader::maybeHotReload() {
    if (programId_ == 0) {
        return false;
    }
    
    // Check if files have been modified
    auto currentVertexTime = getFileLastWriteTime(vertexPath_);
    auto currentFragmentTime = getFileLastWriteTime(fragmentPath_);
    
    if (currentVertexTime > vertexLastWrite_ || currentFragmentTime > fragmentLastWrite_) {
        std::cout << "[shader] reloading " << vertexPath_ << " and " << fragmentPath_ << std::endl;
        
        // Update timestamps
        vertexLastWrite_ = currentVertexTime;
        fragmentLastWrite_ = currentFragmentTime;
        
        // Recompile and relink
        if (compileAndLink()) {
            std::cout << "[shader] reloaded successfully" << std::endl;
            return true;
        } else {
            std::cerr << "[shader] reload failed, keeping previous version" << std::endl;
            return false;
        }
    }
    
    return false;
}

GLint Shader::getUniformLocation(const std::string& name) const {
    if (programId_ == 0) {
        return -1;
    }
    return glGetUniformLocation(programId_, name.c_str());
}

void Shader::bind() const {
    if (programId_ != 0) {
        glUseProgram(programId_);
    }
}

void Shader::unbind() const {
    glUseProgram(0);
}

bool Shader::compileAndLink() {
    // Read vertex shader source
    std::ifstream vertexFile(vertexPath_);
    if (!vertexFile.is_open()) {
        std::cerr << "ERROR: Could not open vertex shader: " << vertexPath_ << std::endl;
        return false;
    }
    
    std::stringstream vertexStream;
    vertexStream << vertexFile.rdbuf();
    std::string vertexSource = vertexStream.str();
    vertexFile.close();
    
    // Read fragment shader source
    std::ifstream fragmentFile(fragmentPath_);
    if (!fragmentFile.is_open()) {
        std::cerr << "ERROR: Could not open fragment shader: " << fragmentPath_ << std::endl;
        return false;
    }
    
    std::stringstream fragmentStream;
    fragmentStream << fragmentFile.rdbuf();
    std::string fragmentSource = fragmentStream.str();
    fragmentFile.close();
    
    // Compile vertex shader
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        return false;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }
    
    // Link program
    if (!linkProgram(vertexShader, fragmentShader)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    // Clean up shaders (they're now linked into the program)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

GLuint Shader::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        std::cerr << "ERROR: Could not create shader" << std::endl;
        return 0;
    }
    
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);
    
    if (!checkShaderCompilation(shader)) {
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    // Clean up previous program if it exists
    cleanup();
    
    // Create program
    programId_ = glCreateProgram();
    if (programId_ == 0) {
        std::cerr << "ERROR: Could not create shader program" << std::endl;
        return false;
    }
    
    // Attach shaders
    glAttachShader(programId_, vertexShader);
    glAttachShader(programId_, fragmentShader);
    
    // Link program
    glLinkProgram(programId_);
    
    // Check linking status
    if (!checkProgramLinking(programId_)) {
        cleanup();
        return false;
    }
    
    return true;
}

bool Shader::checkShaderCompilation(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        
        std::cerr << "ERROR: Shader compilation failed:" << std::endl << log.data() << std::endl;
        return false;
    }
    
    return true;
}

bool Shader::checkProgramLinking(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        
        std::vector<char> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        
        std::cerr << "ERROR: Program linking failed:" << std::endl << log.data() << std::endl;
        return false;
    }
    
    return true;
}

std::chrono::file_clock::time_point Shader::getFileLastWriteTime(const std::string& path) {
    try {
        return std::filesystem::last_write_time(path);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "WARNING: Could not get file time for " << path << ": " << e.what() << std::endl;
        return std::chrono::file_clock::time_point::min();
    }
}

void Shader::cleanup() {
    if (programId_ != 0) {
        glDeleteProgram(programId_);
        programId_ = 0;
    }
}

} // namespace arena::gfx
