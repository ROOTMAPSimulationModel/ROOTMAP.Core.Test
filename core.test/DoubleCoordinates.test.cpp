#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "core/common/DoubleCoordinates.h"

using namespace rootmap;

TEST_CASE("Can be constructed as an (x,y,z) tuple", "[DoubleCoordinates]") {
    auto sut = DoubleCoordinate(123, 456, 789);
    REQUIRE(sut.x == 123);
    REQUIRE(sut.y == 456);
    REQUIRE(sut.z == 789);
}
