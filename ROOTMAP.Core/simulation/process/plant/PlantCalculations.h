#ifndef PlantCalculations_H
#define PlantCalculations_H

#include "core/utility/Vec3d.h"
#include "core/utility/Utility.h"
#include <cmath>

namespace rootmap
{
    const double PI_ON_TWO = PI / 2;
    const double TWO_PI = 2 * PI;
    const double THREE_PI_ON_TWO = 3 * PI / 2;
    const int HALF_RAND_MAX = RAND_MAX / 2;

    void NewRootTipHeading(Vec3d* heading, double deflection, double orientation);
    double RootTipDeflection(const double& deflection_index);
    double RootTipOrientation(const double& geotropism_index);

    inline double RootTipDeflection(const double& deflection_index)
    {
        const double r = Utility::Random1() / 2;

        if ((deflection_index != 0) && (r != 0.0))
        {
            return (TWO_PI * exp(log(r) / deflection_index));
        }

        return 0.0;
    }

    inline double RootTipOrientation(const double& geotropism_index)
    {
        const double r = Utility::Random1();

        // I'm not entirely sure if Utility::Random1() can actually produce a floating-
        // point value of exactly zero but we really can't afford to screw up on the log()
        if ((geotropism_index < 1.0) && (r > 0.0))
        {
            const double angle = PI * exp(log(r) / (1.0 - geotropism_index));
            return ((rand() < HALF_RAND_MAX) ? -angle : angle);
        }

        return (0.0);
    }
} /* namespace rootmap */

#endif // #ifndef PlantCalculations_H
