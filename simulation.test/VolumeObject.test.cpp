#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "simulation/process/modules/VolumeObject.h"

using namespace rootmap;

TEST_CASE("VolumeObject's epsilon is small", "[VolumeObject]") {
    REQUIRE(NEAR_AS_INTERSECTING <= 0.00000001);
}
