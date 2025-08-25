#pragma once
#include <string>
#include <cstdint>

namespace arena::hud {

struct HudStats {
  double fps = 0.0;
  double ms  = 0.0;
  uint64_t ticks = 0;
};

void TextHud_Init();                              // create GL objects, load shaders
void TextHud_Shutdown();                          // delete GL objects
void TextHud_BeginFrame(int fbWidth, int fbHeight); // set viewport-space uniforms
void TextHud_DrawLine(float x, float y, const char* text, float r=1, float g=1, float b=1, float a=1);
void TextHud_DrawStats(const HudStats& s);        // convenience: "FPS: … | ms: … | ticks: …"

} // namespace arena::hud

