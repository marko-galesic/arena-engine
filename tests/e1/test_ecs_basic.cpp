#include <catch2/catch_test_macros.hpp>
#include "arena/ecs/registry.hpp"

TEST_CASE("ECS Registry can be instantiated", "[ecs]") {
    arena::ecs::Registry registry;
    REQUIRE(true); // Basic compilation test passed
}

TEST_CASE("ECS Registry has basic structure", "[ecs]") {
    arena::ecs::Registry registry;
    // This test will be expanded as the Registry implementation grows
    REQUIRE(true);
}
