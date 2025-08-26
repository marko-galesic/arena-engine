#pragma once

#include "arena/gl.hpp"
#include <string>
#include <filesystem>
#include <chrono>

namespace arena::gfx {

class Shader {
public:
    Shader();
    ~Shader();
    
    // Load and compile shader from vertex and fragment files
    bool load(const std::string& basePath);
    
    // Check if shader needs hot-reload and recompile if necessary
    bool maybeHotReload();
    
    // Compile and link shaders (internal use)
    bool compileAndLink();
    
    // Get uniform location
    GLint getUniformLocation(const std::string& name) const;
    
    // Bind shader program
    void bind() const;
    
    // Unbind shader program
    void unbind() const;
    
    // Check if shader is valid
    bool isValid() const { return programId_ != 0; }
    
    // Get program ID
    GLuint getProgramId() const { return programId_; }

private:
    GLuint programId_;
    std::string vertexPath_;
    std::string fragmentPath_;
    std::chrono::file_clock::time_point vertexLastWrite_;
    std::chrono::file_clock::time_point fragmentLastWrite_;
    
    // Compile shader from source
    GLuint compileShader(const std::string& source, GLenum type);
    
    // Link shader program
    bool linkProgram(GLuint vertexShader, GLuint fragmentShader);
    
    // Check shader compilation status
    bool checkShaderCompilation(GLuint shader);
    
    // Check program linking status
    bool checkProgramLinking(GLuint program);
    
    // Get file last write time
    std::chrono::file_clock::time_point getFileLastWriteTime(const std::string& path);
    
    // Cleanup current program
    void cleanup();
};

} // namespace arena::gfx
