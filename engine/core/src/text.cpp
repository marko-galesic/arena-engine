#include "arena/text.hpp"
#include <vector>
#include <string>
#include <cstdio>
#include <cassert>
#include <fstream>
#include <sstream>
#include "arena/gl_api.hpp"
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

/*
 * OpenGL 4.5 Text Rendering System
 * 
 * This implementation uses modern OpenGL 4.5 features:
 * - Direct State Access (DSA) for VAO and VBO operations
 * - External shader loading from files
 * - Modern vertex attribute setup
 * - Efficient buffer management
 * - Support for instanced rendering (future use)
 * - Core profile compatibility
 */

namespace arena::hud {

static GLuint g_prog=0, g_vao=0, g_vbo=0;
static GLint  g_uScreen=-1, g_uColor=-1;
static int g_fbW=1, g_fbH=1;

// OpenGL 4.5: Add support for instanced rendering
static GLuint g_instanceVbo=0;
static bool g_useInstancing = false;

// --- OpenGL 4.5 shader loading helpers
static std::string loadShaderSource(const std::string& filename) {
    // Try multiple paths to find the shader file
    std::vector<std::string> searchPaths = {
        filename,                                    // Direct path
        "../../" + filename,                        // From build/Debug
        "../" + filename,                           // From build
        "../../../" + filename,                     // From build/Debug (alternative)
        "../../../assets/shaders/" + filename.substr(filename.find_last_of('/') + 1), // Just filename
        "assets/shaders/" + filename.substr(filename.find_last_of('/') + 1)          // Just filename
    };
    
    for (const auto& path : searchPaths) {
        std::ifstream file(path);
        if (file.is_open()) {
            printf("Successfully loaded shader from: %s\n", path.c_str());
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
    }
    
    printf("Failed to open shader file: %s (tried multiple paths)\n", filename.c_str());
    return "";
}

static GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[2048];
        glGetShaderInfoLog(shader, 2048, nullptr, log);
        printf("Shader compile failed: %s\n", log);
        assert(false && "shader compile failed");
    }
    return shader;
}

static GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[2048];
        glGetProgramInfoLog(program, 2048, nullptr, log);
        printf("Program link failed: %s\n", log);
        assert(false && "program link failed");
    }
    
    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

void TextHud_Init() {
    printf("TextHud_Init: Starting OpenGL 4.5 initialization\n");
    
    // Load shaders from external files
    std::string vsSource = loadShaderSource("text.vert");
    std::string fsSource = loadShaderSource("text.frag");
    
    if (vsSource.empty() || fsSource.empty()) {
        printf("Failed to load shader files, falling back to embedded shaders\n");
        // Fallback to embedded shaders if file loading fails
        vsSource = R"(#version 450 core
layout(location=0) in vec2 aPosPx;
uniform vec2 uScreen;
void main(){
  vec2 ndc;
  ndc.x = (aPosPx.x / uScreen.x) * 2.0 - 1.0;
  ndc.y = 1.0 - (aPosPx.y / uScreen.y) * 2.0;
  gl_Position = vec4(ndc, 0.0, 1.0);
})";
        fsSource = R"(#version 450 core
out vec4 FragColor;
uniform vec4 uColor;
void main(){ FragColor = uColor; })";
    }
    
    printf("TextHud_Init: Compiling vertex shader\n");
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource);
    printf("TextHud_Init: Compiling fragment shader\n");
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource);
    printf("TextHud_Init: Linking program\n");
    g_prog = linkProgram(vs, fs);
    printf("TextHud_Init: Getting uniform locations\n");
    g_uScreen = glGetUniformLocation(g_prog, "uScreen");
    g_uColor  = glGetUniformLocation(g_prog, "uColor");
    printf("TextHud_Init: uScreen location: %d, uColor location: %d\n", g_uScreen, g_uColor);

    printf("TextHud_Init: Creating VAO and VBO with OpenGL 4.5 DSA\n");
    
    // OpenGL 4.5 Direct State Access for VAO and VBO
    glCreateVertexArrays(1, &g_vao);
    glCreateBuffers(1, &g_vbo);
    
    // Set up vertex buffer with OpenGL 4.5 features
    glNamedBufferData(g_vbo, 64 * 1024, nullptr, GL_STREAM_DRAW);
    
    // Set up vertex array object with DSA
    glEnableVertexArrayAttrib(g_vao, 0);
    glVertexArrayAttribFormat(g_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(g_vao, 0, 0);
    glVertexArrayVertexBuffer(g_vao, 0, g_vbo, 0, 2 * sizeof(float));

    // OpenGL 4.5: Set up instanced rendering for future use
    glCreateBuffers(1, &g_instanceVbo);
    glNamedBufferData(g_instanceVbo, 1024 * sizeof(float), nullptr, GL_STREAM_DRAW);
    
    // Enable alpha blending for text transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    printf("TextHud_Init: OpenGL 4.5 initialization complete\n");
}

void TextHud_Shutdown() {
    if (g_instanceVbo) glDeleteBuffers(1, &g_instanceVbo);
    if (g_vbo) glDeleteBuffers(1, &g_vbo);
    if (g_vao) glDeleteVertexArrays(1, &g_vao);
    if (g_prog){ glDeleteProgram(g_prog); g_prog=0; }
    g_instanceVbo=0; g_vbo=0; g_vao=0;
}

void TextHud_BeginFrame(int fbWidth, int fbHeight) {
    g_fbW = fbWidth; 
    g_fbH = fbHeight;
    
    // Use OpenGL 4.5 Direct State Access for uniform updates
    glProgramUniform2f(g_prog, g_uScreen, (float)g_fbW, (float)g_fbH);
    
    // Bind the program for this frame
    glUseProgram(g_prog);
}

void TextHud_DrawLine(float x, float y, const char* text,
                      float r, float g, float b, float a)
{
    // Stock stb: returns number of QUADS; vertices are (x,y,rgba8), 16-byte stride
    static unsigned char quadBuf[200000]; // ~5000 quads; enlarge if needed
    const int numQuads = stb_easy_font_print(x, y, (char*)text, nullptr,
                                             quadBuf, sizeof(quadBuf));
    if (numQuads <= 0) return;

    // Ensure program is bound
    glUseProgram(g_prog);
    
    // Use OpenGL 4.5 Direct State Access for uniforms
    glProgramUniform4f(g_prog, g_uColor, r, g, b, a);
    
    // Bind VAO once (already set up with DSA)
    glBindVertexArray(g_vao);

    std::vector<float> verts;
    verts.reserve(numQuads * 6 * 2); // 2 tris/quad * 3 verts/tri * 2 floats

    constexpr int STRIDE = 16; // bytes/vertex in stock stb_easy_font
    auto pos = [&](int i) -> const float* {
        return reinterpret_cast<const float*>(quadBuf + i * STRIDE);
    };

    for (int q = 0; q < numQuads; ++q) {
        const int base = q * 4; // 4 verts per quad
        const float* p0 = pos(base + 0);
        const float* p1 = pos(base + 1);
        const float* p2 = pos(base + 2);
        const float* p3 = pos(base + 3);

        // tri 1: 0,1,2
        verts.push_back(p0[0]); verts.push_back(p0[1]);
        verts.push_back(p1[0]); verts.push_back(p1[1]);
        verts.push_back(p2[0]); verts.push_back(p2[1]);
        // tri 2: 0,2,3
        verts.push_back(p0[0]); verts.push_back(p0[1]);
        verts.push_back(p2[0]); verts.push_back(p2[1]);
        verts.push_back(p3[0]); verts.push_back(p3[1]);
    }

    const GLsizeiptr bytes = (GLsizeiptr)(verts.size() * sizeof(float));
    
    // OpenGL 4.5: Use glNamedBufferSubData for direct buffer updates
    glNamedBufferSubData(g_vbo, 0, bytes, verts.data());

    // Draw using the VAO (already bound)
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 2));
}


void TextHud_DrawStats(const HudStats& s) {
    printf("TextHud_DrawStats: Drawing stats - FPS: %.1f, ms: %.2f, ticks: %llu\n", s.fps, s.ms, (unsigned long long)s.ticks);
    
    // Draw a semi-transparent background rectangle for better text readability
    float bgX = 10.0f;
    float bgY = 10.0f;
    float bgW = 300.0f;
    float bgH = 30.0f;
    
    // Background vertices (simple quad)
    std::vector<float> bgVerts = {
        bgX, bgY,
        bgX + bgW, bgY,
        bgX + bgW, bgY + bgH,
        bgX, bgY,
        bgX + bgW, bgY + bgH,
        bgX, bgY + bgH
    };
    
    // Draw background using OpenGL 4.5 DSA
    glProgramUniform4f(g_prog, g_uColor, 0.0f, 0.0f, 0.0f, 0.9f); // Semi-transparent black
    glBindVertexArray(g_vao);
    
    const GLsizeiptr bgBytes = (GLsizeiptr)(bgVerts.size() * sizeof(float));
    glNamedBufferSubData(g_vbo, 0, bgBytes, bgVerts.data());
    glDrawArrays(GL_TRIANGLES, 0, bgVerts.size() / 2);
    
    // Draw the text stats
    char buf[128];
    std::snprintf(buf, sizeof(buf), "FPS: %.1f | ms: %.2f | ticks: %llu",
                  s.fps, s.ms, (unsigned long long)s.ticks);
    
    // Use bright white color for better visibility
    TextHud_DrawLine(10.0f, 20.0f, buf, 1.0f, 1.0f, 1.0f, 1.0f);
    
    printf("TextHud_DrawStats: Draw call complete\n");
}

} // namespace arena::hud
