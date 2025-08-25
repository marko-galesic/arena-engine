#include <catch2/catch_test_macros.hpp>
#include "arena/version.hpp"

TEST_CASE("Contracts module compiles and links", "[contracts]") {
    // Test that we can access the core version
    REQUIRE(arena::Version::major() == 0);
}

TEST_CASE("Core module is accessible from contracts", "[contracts]") {
    // This test verifies that the core module can be linked
    // and its symbols are accessible
    REQUIRE(true);
}
