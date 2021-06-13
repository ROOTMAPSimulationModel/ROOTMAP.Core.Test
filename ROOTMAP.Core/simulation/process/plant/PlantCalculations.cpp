#include "simulation/process/plant/PlantCalculations.h"
#include "core/common/Constants.h"
#include <cstdlib>


namespace rootmap
{
    long int number_of_NewHeadings = 0;


    void NewRootTipHeading
    (Vec3d* heading,
        double deflection,
        double orientation)
    {
        const double old_heading_x = heading->x;
        const double old_heading_y = heading->y;
        const double old_heading_z = -heading->z;

        const double rotated_heading_x = sin(deflection) * cos(orientation);
        const double rotated_heading_y = sin(deflection) * sin(orientation);
        const double rotated_heading_z = cos(deflection);

        /* FIRST, Calculate Phi */
        double phi;
        if (old_heading_z == 1)
        {
            phi = 0.0;
        }
        else if (old_heading_z == -1)
        {
            phi = PI;
        }
        else if (old_heading_z == 0)
        {
            phi = PI_ON_TWO;
        }
        else
        {
            phi = atan(sqrt(old_heading_x * old_heading_x + old_heading_y * old_heading_y) / old_heading_z);
            if (old_heading_z < 0)
            {
                phi += PI;
            }
        }

        //Calculate Theta.
        //Separating the loops may come with a slight performance penalty, however
        //a) it is VERY slight if at all
        //b) it is unlikely
        //c) the gain in understanding code flow makes up for this
        //
        double theta;
        if ((old_heading_z == 1) || (old_heading_z == -1))
        {
            theta = Utility::Random1() * TWO_PI;
        }
        else if (old_heading_x == 0)
        {
            theta = (old_heading_y > 0) ? PI_ON_TWO : THREE_PI_ON_TWO;
        }
        else
        {
            theta = atan(old_heading_y / old_heading_x);
            if (old_heading_x < 0)
            {
                (old_heading_y >= 0) ? theta += PI : theta -= PI;
            }
        }

        const double cos_phi = cos(phi);
        const double cos_theta = cos(theta);
        const double sin_phi = sin(phi);
        const double sin_theta = sin(theta);

        const double heading_x_1 = rotated_heading_x * cos_phi * cos_theta;
        const double heading_x_2 = rotated_heading_y * ((-old_heading_y * sin_phi) -
            (old_heading_z * cos_phi * sin_theta));
        const double heading_x_3 = rotated_heading_z * old_heading_x;
        const double heading_x = heading_x_1 + heading_x_2 + heading_x_3;

        heading->x = heading_x;

        const double heading_y_1 = rotated_heading_x * cos_phi * sin_theta;
        const double heading_y_2 = rotated_heading_y * ((old_heading_x * sin_phi) +
            (old_heading_z * cos_phi * cos_theta));
        const double heading_y_3 = rotated_heading_z * old_heading_y;
        const double heading_y = heading_y_1 + heading_y_2 + heading_y_3;

        heading->y = heading_y;

        const double heading_z_1 = rotated_heading_x * (-sin_phi);
        const double heading_z_2 = rotated_heading_y * ((old_heading_x * cos_phi * sin_theta) -
            (old_heading_y * cos_phi * cos_theta));
        const double heading_z_3 = rotated_heading_z * old_heading_z;
        const double heading_z = heading_z_1 + heading_z_2 + heading_z_3;

        heading->z = -heading_z;

        ++number_of_NewHeadings;
    }
} /* namespace rootmap */


