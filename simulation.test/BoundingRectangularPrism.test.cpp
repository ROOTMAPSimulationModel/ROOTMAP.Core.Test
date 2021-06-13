#include "catch.hpp"
#include "simulation/process/modules/BoundingRectangularPrism.h"
#include "simulation/process/modules/BoundingCylinder.h"
using namespace rootmap;

TEST_CASE("BoundingRectangularPrism can be constructed", "[BoundingRectangularPrism]") {
    // TODO: a test case that fails until the constructor will barf if any of those RPP (Root Penetration Probability) parameters fall outside [0,1].
    auto sut = new BoundingRectangularPrism(42, DoubleCoordinate(), DoubleCoordinate(), "blah", 123, 456, 789, 101112, 131415, 161718);
    REQUIRE(sut != __nullptr);
}

SCENARIO("Using BoundingRectangularPrism::Contains function", "[BoundingRectangularPrism]") {
    GIVEN("A BoundingRectangularPrism is defined") {
        // A unit cube.
        auto sut = new BoundingRectangularPrism(42, DoubleCoordinate(0, 0, 0), DoubleCoordinate(1, 1, 1), "blah", 123, 456, 789, 101112, 131415, 161718);
        WHEN("A point inside the prism is tested") {
            auto dc = DoubleCoordinate(0.5, 0.5, 0.5);
            THEN("Contains() returns true") {
                REQUIRE(sut->Contains(dc) == true);
            }
        }
        WHEN("A point below the prism is tested") {
            auto dc = DoubleCoordinate(0.5, 0.5, 1.5);
            THEN("Contains() returns false") {
                REQUIRE(sut->Contains(dc) == false);
            }
        }
        WHEN("A point above the prism is tested") {
            auto dc = DoubleCoordinate(0.5, 0.5, -0.1);
            THEN("Contains() returns false") {
                REQUIRE(sut->Contains(dc) == false);
            }
        }
        WHEN("A point outside and alongside the prism is tested") {
            auto dc = DoubleCoordinate(1.5, 1, 0.5);
            THEN("Contains() returns false") {
                REQUIRE(sut->Contains(dc) == false);
            }
        }
        WHEN("A point just inside the prism is tested") {
            auto doubleEpsilon = NEAR_AS_INTERSECTING * 2;
            auto dc = DoubleCoordinate(doubleEpsilon, doubleEpsilon, doubleEpsilon);
            THEN("Contains() returns true") {
                REQUIRE(sut->Contains(dc) == true);
            }
        }
        WHEN("A point just outside the prism is tested") {
            auto doubleEpsilon = NEAR_AS_INTERSECTING * 2;
            auto dc = DoubleCoordinate(-doubleEpsilon, -doubleEpsilon, -doubleEpsilon);
            THEN("Contains() returns false") {
                REQUIRE(sut->Contains(dc) == false);
            }
        }
    }
}

SCENARIO("Testing if BoundingRectangularPrism is inside another BoundingRectangularPrism", "[BoundingRectangularPrism]") {
    GIVEN("A BoundingRectangularPrism A is defined") {
        // A unit cube.
        auto sut = new BoundingRectangularPrism(42, DoubleCoordinate(0, 0, 0), DoubleCoordinate(1, 1, 1), "blah", 123, 456, 789, 101112, 131415, 161718);
        WHEN("An enclosing BoundingRectangularPrism B is tested") {
            auto enclosingRect = new BoundingRectangularPrism(1, DoubleCoordinate(-1, -1, -1), DoubleCoordinate(2, 2, 2), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(B) returns true") {
                REQUIRE(sut->Within(enclosingRect) == true);
            }
        }
        WHEN("An only-just enclosing BoundingRectangularPrism C is tested") {
            auto doubleEpsilon = NEAR_AS_INTERSECTING * 2;
            auto enclosingRect = new BoundingRectangularPrism(1, DoubleCoordinate(-doubleEpsilon, -doubleEpsilon, -doubleEpsilon), DoubleCoordinate(1 + doubleEpsilon, 1 + doubleEpsilon, 1 + doubleEpsilon), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(C) returns true") {
                REQUIRE(sut->Within(enclosingRect) == true);
            }
        }
        WHEN("An identical and coincident BoundingRectangularPrism D is tested") {
            auto coincidentRect = new BoundingRectangularPrism(1, DoubleCoordinate(0, 0, 0), DoubleCoordinate(1, 1, 1), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(D) returns false") {
                REQUIRE(sut->Within(coincidentRect) == false);
            }
        }
        WHEN("An enclosed BoundingRectangularPrism E is tested") {
            auto enclosedRect = new BoundingRectangularPrism(1, DoubleCoordinate(0.2, 0.2, 0.2), DoubleCoordinate(0.8, 0.8, 0.8), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(E) returns false") {
                REQUIRE(sut->Within(enclosedRect) == false);
            }
        }
        WHEN("An intersecting BoundingRectangularPrism F is tested") {
            auto intersectingRect = new BoundingRectangularPrism(1, DoubleCoordinate(-1, 0.5, 0), DoubleCoordinate(0.1, 0.6, 0.5), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(F) returns false") {
                REQUIRE(sut->Within(intersectingRect) == false);
            }
        }
        WHEN("A non-intersecting, non-coincident BoundingRectangularPrism G is tested") {
            auto separateRect = new BoundingRectangularPrism(1, DoubleCoordinate(5, 5, 5), DoubleCoordinate(6, 6, 6), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(G) returns false") {
                REQUIRE(sut->Within(separateRect) == false);
            }
        }
    }
}

SCENARIO("Testing if BoundingRectangularPrism is inside a BoundingCylinder", "[BoundingRectangularPrism]") {
    GIVEN("A BoundingRectangularPrism A is defined") {
        // A unit cube.
        auto sut = new BoundingRectangularPrism(42, DoubleCoordinate(0, 0, 0), DoubleCoordinate(1, 1, 1), "blah", 123, 456, 789, 101112, 131415, 161718);
        auto epsilon = NEAR_AS_INTERSECTING;
        auto intersectingRadius = sqrt(0.5 * 0.5 + 0.5 * 0.5);

        WHEN("An enclosing BoundingCylinder B is tested") {
            auto minimalEnclosingRadius = intersectingRadius + epsilon;
            auto enclosingCyl = new BoundingCylinder(42, DoubleCoordinate(0.5, 0.5, -epsilon), 1 + 2 * epsilon, minimalEnclosingRadius, "fake algo", 77, 88, 99, 111, 222, 333);
            THEN("A->Within(B) returns true") {
                REQUIRE(sut->Within(enclosingCyl) == true);
            }
        }
        WHEN("An almost enclosing BoundingCylinder C is tested") {
            auto almostEnclosingCyl = new BoundingCylinder(42, DoubleCoordinate(0.5, 0.5, -epsilon), 1 + 2 * epsilon, intersectingRadius - epsilon, "fake algo", 77, 88, 99, 111, 222, 333);
            THEN("A->Within(C) returns false") {
                REQUIRE(sut->Within(almostEnclosingCyl) == false);
            }
        }
        WHEN("An enclosed BoundingCylinder D is tested") {
            auto maximalEnclosedRadius = intersectingRadius - epsilon;
            auto enclosedCyl = new BoundingCylinder(42, DoubleCoordinate(0.5, 0.5, epsilon), 1 - 2 * epsilon, maximalEnclosedRadius, "fake algo", 77, 88, 99, 111, 222, 333);
            THEN("A->Within(D) returns false") {
                REQUIRE(sut->Within(enclosedCyl) == false);
            }
        }
        WHEN("An almost enclosed BoundingCylinder E is tested") {
            auto almostEnclosedCyl = new BoundingCylinder(42, DoubleCoordinate(0.5, 0.5, epsilon), 1 - 2 * epsilon, intersectingRadius, "fake algo", 77, 88, 99, 111, 222, 333);
            THEN("A->Within(E) returns false") {
                REQUIRE(sut->Within(almostEnclosedCyl) == false);
            }
        }
        WHEN("An intersecting BoundingCylinder F is tested") {
            auto intersectingCyl = new BoundingCylinder(42, DoubleCoordinate(0, 0, 0), 1, 1, "fake algo", 77, 88, 99, 111, 222, 333);
            THEN("A->Within(F) returns false") {
                REQUIRE(sut->Within(intersectingCyl) == false);
            }
        }
        WHEN("A non-intersecting, non-coincident BoundingCylinder G is tested") {
            auto separateCyl = new BoundingCylinder(42, DoubleCoordinate(5, 5, 5), 1, 1, "fake algo", 77, 88, 99, 111, 222, 333);
            THEN("A->Within(G) returns false") {
                REQUIRE(sut->Within(separateCyl) == false);
            }
        }
    }
}
