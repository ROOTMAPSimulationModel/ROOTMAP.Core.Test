#include "catch.hpp"
#include "simulation/process/modules/BoundingCylinder.h"
#include "simulation/process/modules/BoundingRectangularPrism.h"
using namespace rootmap;

TEST_CASE("BoundingCylinder can be constructed", "[BoundingCylinder]") {
    auto sut = new BoundingCylinder(42, DoubleCoordinate(), 55, 66, "fake algo", 77, 88, 99, 111, 222, 333);
    REQUIRE(sut != __nullptr);
}

auto epsilon = NEAR_AS_INTERSECTING;

SCENARIO("Using BoundingCylinder::Contains function", "[BoundingCylinder]") {
    GIVEN("A BoundingCylinder is defined") {
        auto sut = new BoundingCylinder(42, DoubleCoordinate(0, 0, 0), 1, 1, "fake algo", 77, 88, 99, 111, 222, 333);
        WHEN("A point inside the cylinder is tested") {
            auto dc = DoubleCoordinate(0.25, 0.25, 0.5);
            THEN("Contains() returns true") {
                REQUIRE(sut->Contains(dc) == true);
            }
        }
        WHEN("A point below the cylinder is tested") {
            auto dc = DoubleCoordinate(0, 0, 1.5);
            THEN("Contains() returns false") {
                REQUIRE(sut->Contains(dc) == false);
            }
        }
        WHEN("A point above the cylinder is tested") {
            auto dc = DoubleCoordinate(0, 0, -0.1);
            THEN("Contains() returns false") {
                REQUIRE(sut->Contains(dc) == false);
            }
        }
        WHEN("A point outside and alongside the cylinder is tested") {
            auto dc = DoubleCoordinate(0, 1, 0.5);
            THEN("Contains() returns false") {
                REQUIRE(sut->Contains(dc) == false);
            }
        }

        WHEN("A point just inside the cylinder's top face is tested") {
            auto dc = DoubleCoordinate(0, 0, epsilon);
            THEN("Contains() returns true") {
                REQUIRE(sut->Contains(dc) == true);
            }
        }
        WHEN("A point just above the cylinder is tested") {
            auto dc = DoubleCoordinate(0, 0, -epsilon);
            THEN("Contains() returns false") {
                REQUIRE(sut->Contains(dc) == false);
            }
        }
        WHEN("Points just inside the curved wall of the cylinder are tested") {
            auto dc1 = DoubleCoordinate(1 - epsilon, 0, 0.5);
            auto xOnCircle = 0.2;
            auto yOnCircle = sqrt(1 - xOnCircle * xOnCircle);
            auto dc2 = DoubleCoordinate(xOnCircle - epsilon, yOnCircle, 0.5);
            THEN("Contains() returns true") {
                REQUIRE(sut->Contains(dc1) == true);
                REQUIRE(sut->Contains(dc2) == true);
            }
        }
        WHEN("Points just outside the curved wall of the cylinder are tested") {
            auto dc1 = DoubleCoordinate(1 + epsilon, 0, 0.5);
            auto xOnCircle = 0.8;
            auto yOnCircle = sqrt(1 - xOnCircle * xOnCircle);
            auto dc2 = DoubleCoordinate(xOnCircle + epsilon, yOnCircle, 0.5);
            THEN("Contains() returns false") {
                REQUIRE(sut->Contains(dc1) == false);
                REQUIRE(sut->Contains(dc2) == false);
            }
        }
    }
}

SCENARIO("Testing if BoundingCylinder is inside another BoundingCylinder", "[BoundingCylinder]") {
    GIVEN("A BoundingCylinder A is defined") {
        auto sut = new BoundingCylinder(42, DoubleCoordinate(0, 0, 0), 1, 1, "blah", 123, 456, 789, 101112, 131415, 161718);
        WHEN("An enclosing BoundingCylinder B is tested") {
            auto enclosingCyl = new BoundingCylinder(1, DoubleCoordinate(0, 0, -5), 20, 2, "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(B) returns true") {
                REQUIRE(sut->Within(enclosingCyl) == true);
            }
        }
        WHEN("An only-just enclosing BoundingCylinder C is tested") {
            auto enclosingCyl = new BoundingCylinder(1, DoubleCoordinate(0, 0, -epsilon), 1 + 2 * epsilon, 1 + epsilon, "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(C) returns true") {
                REQUIRE(sut->Within(enclosingCyl) == true);
            }
        }
        WHEN("An identical and coincident BoundingCylinder D is tested") {
            auto coincidentCyl = new BoundingCylinder(1, DoubleCoordinate(0, 0, 0), 1, 1, "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(D) returns false") {
                REQUIRE(sut->Within(coincidentCyl) == false);
            }
        }
        WHEN("An enclosed BoundingCylinder E is tested") {
            auto enclosedCyl = new BoundingCylinder(1, DoubleCoordinate(0, 0, epsilon), 1 - 2 * epsilon, 1 - epsilon, "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(E) returns false") {
                REQUIRE(sut->Within(enclosedCyl) == false);
            }
        }
        WHEN("An intersecting BoundingCylinder F is tested") {
            auto intersectingCyl = new BoundingCylinder(42, DoubleCoordinate(0.2, 0.2, 0.5), 0.1, 1, "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(F) returns false") {
                REQUIRE(sut->Within(intersectingCyl) == false);
            }
        }
        WHEN("A non-intersecting, non-coincident BoundingCylinder G is tested") {
            auto separateCyl = new BoundingCylinder(42, DoubleCoordinate(1, 2, 3), 10, 1, "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(G) returns false") {
                REQUIRE(sut->Within(separateCyl) == false);
            }
        }
    }
}

SCENARIO("Testing if BoundingCylinder is inside a BoundingRectangularPrism", "[BoundingCylinder]") {
    GIVEN("A BoundingCylinder A is defined") {
        auto sut = new BoundingCylinder(42, DoubleCoordinate(1, 1, 0), 1, 1, "blah", 123, 456, 789, 101112, 131415, 161718);

        WHEN("An enclosing BoundingRectangularPrism B is tested") {
            auto enclosingRect = new BoundingRectangularPrism(1, DoubleCoordinate(-epsilon, -epsilon, -epsilon), DoubleCoordinate(2 + epsilon, 2 + epsilon, 2 + epsilon), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(B) returns true") {
                REQUIRE(sut->Within(enclosingRect) == true);
            }
        }
        WHEN("An almost enclosing BoundingRectangularPrism C is tested") {
            auto almostEnclosingRect = new BoundingRectangularPrism(1, DoubleCoordinate(0, 0, 0), DoubleCoordinate(2, 2, 1), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(C) returns false") {
                REQUIRE(sut->Within(almostEnclosingRect) == false);
            }
        }
        WHEN("An enclosed BoundingRectangularPrism D is tested") {
            auto enclosedRect = new BoundingRectangularPrism(1, DoubleCoordinate(1, 1, epsilon), DoubleCoordinate(1.5, 1.5, 1 - epsilon), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(D) returns false") {
                REQUIRE(sut->Within(enclosedRect) == false);
            }
        }
        WHEN("An almost enclosed BoundingRectangularPrism E is tested") {
            auto almostEnclosedRect = new BoundingRectangularPrism(1, DoubleCoordinate(1, 1, epsilon), DoubleCoordinate(1.75, 1.75, 0.5), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(E) returns false") {
                REQUIRE(sut->Within(almostEnclosedRect) == false);
            }
        }
        WHEN("An intersecting BoundingRectangularPrism F is tested") {
            auto intersectingRect = new BoundingRectangularPrism(1, DoubleCoordinate(0, 0, 0), DoubleCoordinate(1, 1, 1), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(F) returns false") {
                REQUIRE(sut->Within(intersectingRect) == false);
            }
        }
        WHEN("A non-intersecting, non-coincident BoundingRectangularPrism G is tested") {
            auto separateRect = new BoundingRectangularPrism(1, DoubleCoordinate(10, 10, 10), DoubleCoordinate(11, 11, 11), "blah", 123, 456, 789, 101112, 131415, 161718);
            THEN("A->Within(G) returns false") {
                REQUIRE(sut->Within(separateRect) == false);
            }
        }
    }
}
