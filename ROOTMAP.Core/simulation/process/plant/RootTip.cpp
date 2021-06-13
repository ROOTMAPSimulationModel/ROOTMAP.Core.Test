#include "simulation/process/plant/RootTip.h"
#include "simulation/process/plant/PlantCalculations.h"
#include "simulation/common/IdentifierUtility.h"
// #include "simulation/process/plant/Plant.h"
#include "core/common/Constants.h"


namespace rootmap
{
    RootTip::RootTip()
        : roottip_position(0.0, 0.0, 0.0)
        , roottip_age(0.0)
        , roottip_length(0.0)
        , roottip_heading(0.0, 0.0, 0.0)
        , roottip_dead(false)
        , m_id(IdentifierUtility::instance().useNextPlantElementIdentifier())
    {
    }

    RootTip::RootTip(const DoubleCoordinate& position,
        const double& start_age,
        const Vec3d& direction,
        const long& branch_order,
        const double& initial_branch_angle,
        const double& radius,
        const AxisType& axis_type
    )
        : roottip_position(position)
        , roottip_age(start_age)
        , roottip_length(0.0)
        , roottip_heading(direction)
        , roottip_type(axis_type)
        , roottip_order(branch_order)
        , roottip_radius(radius)
        // MSA 11.04.05 Argh! This boolean was previously NOT INITIALISED in this constructor!
        , roottip_dead(false)
        , m_id(IdentifierUtility::instance().useNextPlantElementIdentifier())
    {
        if (branch_order > 0)
        {
            NewRootTipHeading(&roottip_heading, initial_branch_angle * PI / 180, RootTipOrientation(0));
        }
    }
} /* namespace rootmap */


