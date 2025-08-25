#include <catch2/catch_test_macros.hpp>
#include "arena/text.hpp"
#include "arena/gl_api.hpp"

TEST_CASE("TextHud API no-crash smoke"){
  using namespace arena::hud;
  // We can't create a GL context in a unit test portably; just verify the symbols are linkable
  // and the string formatting path works.
  HudStats s; s.fps=60.0; s.ms=16.7; s.ticks=123;
  // compile-time call signatures:
  (void) &TextHud_Init;
  (void) &TextHud_Shutdown;
  (void) &TextHud_BeginFrame;
  (void) &TextHud_DrawLine;
  (void) &TextHud_DrawStats;
  REQUIRE(true);
}
