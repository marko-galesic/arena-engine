#include "arena/text.hpp"
#include <vector>
#include <string>
#include <cstdio>
#include <cassert>
#include "arena/gl_api.hpp"
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

namespace arena::hud {

static GLuint g_prog=0, g_vao=0, g_vbo=0;
static GLint  g_uScreen=-1, g_uColor=-1;
static int g_fbW=1, g_fbH=1;

// --- helpers
static GLuint compile(GLenum type, const char* src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);
  GLint ok=0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if(!ok){ 
    char log[2048]; 
    glGetShaderInfoLog(s, 2048, nullptr, log); 
    printf("Shader compile failed: %s\n", log);
    assert(false && "shader compile failed"); 
  }
  return s;
}
static GLuint link(GLuint vs, GLuint fs){
  GLuint p=glCreateProgram();
  glAttachShader(p,vs); glAttachShader(p,fs);
  glLinkProgram(p);
  GLint ok=0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
  if(!ok){ 
    char log[2048]; 
    glGetProgramInfoLog(p, 2048, nullptr, log); 
    printf("Program link failed: %s\n", log);
    assert(false && "program link failed"); 
  }
  glDetachShader(p,vs); glDetachShader(p,fs);
  glDeleteShader(vs); glDeleteShader(fs);
  return p;
}

void TextHud_Init() {
  printf("TextHud_Init: Starting initialization\n");
  
  // Load shader sources from disk here if you prefer; embedding for brevity:
  // Using version 330 for better compatibility
  const char* vsrc =
R"(#version 330 core
layout(location=0) in vec2 aPosPx;         // pixel space (origin at top-left input)
uniform vec2 uScreen;                      // framebuffer size in pixels
void main(){
  // convert pixel coords to NDC; (0,0) top-left -> (-1,+1) in NDC
  vec2 ndc;
  ndc.x = (aPosPx.x / uScreen.x) * 2.0 - 1.0;
  ndc.y = 1.0 - (aPosPx.y / uScreen.y) * 2.0;
  gl_Position = vec4(ndc, 0.0, 1.0);
})";

  const char* fsrc =
R"(#version 330 core
out vec4 FragColor;
uniform vec4 uColor;
void main(){ FragColor = uColor; })";

  printf("TextHud_Init: Compiling vertex shader\n");
  GLuint vs = compile(GL_VERTEX_SHADER, vsrc);
  printf("TextHud_Init: Compiling fragment shader\n");
  GLuint fs = compile(GL_FRAGMENT_SHADER, fsrc);
  printf("TextHud_Init: Linking program\n");
  g_prog = link(vs, fs);
  printf("TextHud_Init: Getting uniform locations\n");
  g_uScreen = glGetUniformLocation(g_prog, "uScreen");
  g_uColor  = glGetUniformLocation(g_prog, "uColor");
  printf("TextHud_Init: uScreen location: %d, uColor location: %d\n", g_uScreen, g_uColor);

  printf("TextHud_Init: Creating VAO and VBO\n");
  glCreateVertexArrays(1, &g_vao);
  glCreateBuffers(1, &g_vbo);
  
  // Set up vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
  glBufferData(GL_ARRAY_BUFFER, 64 * 1024, nullptr, GL_STREAM_DRAW);
  
  // Set up vertex array object
  glBindVertexArray(g_vao);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

  // enable alpha blending once (text is solid quads but allow alpha)
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  printf("TextHud_Init: Initialization complete\n");
}

void TextHud_Shutdown() {
  if (g_vbo) glDeleteBuffers(1, &g_vbo);
  if (g_vao) glDeleteVertexArrays(1, &g_vao);
  if (g_prog){ glDeleteProgram(g_prog); g_prog=0; }
  g_vbo=0; g_vao=0;
}

void TextHud_BeginFrame(int fbWidth, int fbHeight) {
  g_fbW = fbWidth; g_fbH = fbHeight;
  glUseProgram(g_prog);
  glUniform2f(g_uScreen, (float)g_fbW, (float)g_fbH);
}

void TextHud_DrawLine(float x, float y, const char* text,
                      float r, float g, float b, float a)
{
    // Stock stb: returns number of QUADS; vertices are (x,y,rgba8), 16-byte stride
    static unsigned char quadBuf[200000]; // ~5000 quads; enlarge if needed
    const int numQuads = stb_easy_font_print(x, y, (char*)text, nullptr,
                                             quadBuf, sizeof(quadBuf));
    if (numQuads <= 0) return;



    glUseProgram(g_prog);
    glUniform4f(g_uColor, r, g, b, a);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

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
    GLint64 current = 0;
    glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &current);
    if (bytes > current) glBufferData(GL_ARRAY_BUFFER, bytes, nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, verts.data());

    // VAO is already: location 0 -> vec2, tightly packed (2*sizeof(float))
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 2));
}


void TextHud_DrawStats(const HudStats& s) {
  printf("TextHud_DrawStats: Drawing stats - FPS: %.1f, ms: %.2f, ticks: %llu\n", s.fps, s.ms, (unsigned long long)s.ticks);
  
  // Draw a semi-transparent background rectangle for better text readability
  // Simple quad for background (x, y, width, height)
  float bgX = 5.0f;
  float bgY = 5.0f;
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
  
  // Draw background
  glUseProgram(g_prog);
  glUniform4f(g_uColor, 0.0f, 0.0f, 0.0f, 0.7f); // Semi-transparent black
  glBindVertexArray(g_vao);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
  
  const GLsizeiptr bgBytes = (GLsizeiptr)(bgVerts.size() * sizeof(float));
  glBufferSubData(GL_ARRAY_BUFFER, 0, bgBytes, bgVerts.data());
  glDrawArrays(GL_TRIANGLES, 0, bgVerts.size() / 2);
  
  // Draw the text stats
  char buf[128];
  std::snprintf(buf, sizeof(buf), "FPS: %.1f | ms: %.2f | ticks: %llu",
                s.fps, s.ms, (unsigned long long)s.ticks);
  
  // Use bright green color for better visibility
  TextHud_DrawLine(10.0f, 20.0f, buf, 0.0f, 1.0f, 0.0f, 1.0f);
  
  printf("TextHud_DrawStats: Draw call complete\n");
}

} // namespace arena::hud
