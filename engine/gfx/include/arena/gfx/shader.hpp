#pragma once

#include "arena/gl.hpp"
#include <string>
#include <filesystem>

namespace arena::gfx {

struct Shader {
    std::string vsPath, fsPath; 
    GLuint prog = 0;
    std::filesystem::file_time_type vsM, fsM;
    
    bool load(const char* vs, const char* fs);
    bool reloadIfChanged(); // checks mtimes, recompiles/link if changed
    void use() const;
    GLint uni(const char* name) const;

private:
    // Helper methods for compilation and linking
    bool compileAndLink();
    GLuint compileShader(const std::string& source, GLenum type);
    bool linkProgram(GLuint vsShader, GLuint fsShader);
    bool checkShaderCompilation(GLuint shader);
    bool checkProgramLinking(GLuint program);
};

} // namespace arena::gfx
