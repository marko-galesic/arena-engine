#include <catch2/catch_all.hpp>

extern "C" int RunHeadlessForMs(int ms, int tickHz);

TEST_CASE("Headless 1s ~ 60 ticks") {
    int ticks = RunHeadlessForMs(1000, 60);
    REQUIRE(ticks >= 59);
    REQUIRE(ticks <= 61);
}
