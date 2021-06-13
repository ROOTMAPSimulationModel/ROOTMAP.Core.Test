#ifndef RootTip_H
#define RootTip_H
// #include "core/common/Structures.h"
#include "core/common/DoubleCoordinates.h"
#include "simulation/common/Types.h"
#include <list>

namespace rootmap
{
    class RootTip
    {
    public:
        RootTip();
        RootTip(const DoubleCoordinate& position,
            const double& start_age,
            const Vec3d& direction,
            const long& branch_order,
            const double& initial_branch_angle,
            const double& radius,
            const AxisType& axis_type
        );

    public:
        DoubleCoordinate roottip_position;
        std::list<DoubleCoordinate> roottip_midpoints;
        Vec3d roottip_heading;
        double roottip_length;
        double roottip_age;
        long roottip_type;
        long roottip_order;
        double roottip_radius;
        // MSA 09.11.16 Converted to bool
        bool roottip_dead; //vmd 25/1/99 if the root tip is dead (dried out or what ever) then this will be 1 otherwise it is 0.
        PlantElementIdentifier m_id;
    };
} /* namespace rootmap */

#endif // #ifndef RootTip_H
