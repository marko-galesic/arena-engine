#include "arena/gfx/shader.hpp"
#include <glad/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace arena::gfx {

bool Shader::load(const char* vs, const char* fs) {
    vsPath = vs;
    fsPath = fs;
    
    // Try multiple paths to find the shader files
    std::vector<std::string> vsSearchPaths = {
        vsPath,                                    // Direct path
        "../../" + vsPath,                        // From build/Debug
        "../" + vsPath,                           // From build
        "../../../" + vsPath,                     // From build/Debug (alternative)
        "../../../assets/shaders/" + std::filesystem::path(vsPath).filename().string(), // Just filename
        "assets/shaders/" + std::filesystem::path(vsPath).filename().string()          // Just filename
    };
    
    std::vector<std::string> fsSearchPaths = {
        fsPath,                                    // Direct path
        "../../" + fsPath,                        // From build/Debug
        "../" + fsPath,                           // From build
        "../../../" + fsPath,                     // From build/Debug (alternative)
        "../../../assets/shaders/" + std::filesystem::path(fsPath).filename().string(), // Just filename
        "assets/shaders/" + std::filesystem::path(fsPath).filename().string()          // Just filename
    };
    
    // Find the first valid vertex shader path
    std::string foundVsPath;
    for (const auto& path : vsSearchPaths) {
        if (std::filesystem::exists(path)) {
            foundVsPath = path;
            break;
        }
    }
    
    // Find the first valid fragment shader path
    std::string foundFsPath;
    for (const auto& path : fsSearchPaths) {
        if (std::filesystem::exists(path)) {
            foundFsPath = path;
            break;
        }
    }
    
    if (foundVsPath.empty() || foundFsPath.empty()) {
        std::cerr << "ERROR: Shader files not found: " << vsPath << " or " << fsPath << std::endl;
        std::cerr << "Tried paths:" << std::endl;
        for (const auto& path : vsSearchPaths) {
            std::cerr << "  VS: " << path << std::endl;
        }
        for (const auto& path : fsSearchPaths) {
            std::cerr << "  FS: " << path << std::endl;
        }
        return false;
    }
    
    // Update paths to the found ones
    vsPath = foundVsPath;
    fsPath = foundFsPath;
    
    // Get initial file timestamps
    vsM = std::filesystem::last_write_time(vsPath);
    fsM = std::filesystem::last_write_time(fsPath);
    
    // Load and compile shaders
    return compileAndLink();
}

bool Shader::reloadIfChanged() {
    if (prog == 0) {
        return false;
    }
    
    // Check if files have been modified
    auto currentVsTime = std::filesystem::last_write_time(vsPath);
    auto currentFsTime = std::filesystem::last_write_time(fsPath);
    
    if (currentVsTime > vsM || currentFsTime > fsM) {
        std::cout << "[shader] reloading " << vsPath << " and " << fsPath << std::endl;
        
        // Update timestamps
        vsM = currentVsTime;
        fsM = currentFsTime;
        
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

void Shader::use() const {
    if (prog != 0) {
        glUseProgram(prog);
    }
}

GLint Shader::uni(const char* name) const {
    if (prog == 0) {
        return -1;
    }
    return glGetUniformLocation(prog, name);
}

bool Shader::compileAndLink() {
    // Read vertex shader source
    std::ifstream vsFile(vsPath);
    if (!vsFile.is_open()) {
        std::cerr << "ERROR: Could not open vertex shader: " << vsPath << std::endl;
        return false;
    }
    
    std::stringstream vsStream;
    vsStream << vsFile.rdbuf();
    std::string vsSource = vsStream.str();
    vsFile.close();
    
    // Read fragment shader source
    std::ifstream fsFile(fsPath);
    if (!fsFile.is_open()) {
        std::cerr << "ERROR: Could not open fragment shader: " << fsPath << std::endl;
        return false;
    }
    
    std::stringstream fsStream;
    fsStream << fsFile.rdbuf();
    std::string fsSource = fsStream.str();
    fsFile.close();
    
    // Compile vertex shader
    GLuint vsShader = compileShader(vsSource, GL_VERTEX_SHADER);
    if (vsShader == 0) {
        return false;
    }
    
    // Compile fragment shader
    GLuint fsShader = compileShader(fsSource, GL_FRAGMENT_SHADER);
    if (fsShader == 0) {
        glDeleteShader(vsShader);
        return false;
    }
    
    // Link program
    if (!linkProgram(vsShader, fsShader)) {
        glDeleteShader(vsShader);
        glDeleteShader(fsShader);
        return false;
    }
    
    // Clean up shaders (they're now linked into the program)
    glDeleteShader(vsShader);
    glDeleteShader(fsShader);
    
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

bool Shader::linkProgram(GLuint vsShader, GLuint fsShader) {
    // Clean up previous program if it exists
    if (prog != 0) {
        glDeleteProgram(prog);
        prog = 0;
    }
    
    // Create program
    prog = glCreateProgram();
    if (prog == 0) {
        std::cerr << "ERROR: Could not create shader program" << std::endl;
        return false;
    }
    
    // Attach shaders
    glAttachShader(prog, vsShader);
    glAttachShader(prog, fsShader);
    
    // Link program
    glLinkProgram(prog);
    
    // Check linking status
    if (!checkProgramLinking(prog)) {
        glDeleteProgram(prog);
        prog = 0;
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

} // namespace arena::gfx
