#include "simulation/process/plant/Branch.h"
#include "simulation/common/IdentifierUtility.h"
#include "core/utility/Utility.h"


namespace rootmap
{
    Branch::Branch()
        : m_position(0, 0, 0)
        , m_length(0.0)
        , m_age(0.0)
        , m_child(0)
        , m_childBranch(0)
        , m_parentBranch(0)
        , m_id(IdentifierUtility::instance().useNextPlantElementIdentifier())
    {
    }

    Branch::Branch
    (DoubleCoordinate* position,
        const double& length,
        const double& start_age,
        RootTip* child,
        Branch* childBranch,
        Branch* parent_branch,
        const AxisType& axis_type,
        const long& order,
        const double& radius
    )
        : m_position(*position)
        , m_length(length)
        , m_age(start_age)
        , m_child(child)
        , m_childBranch(childBranch)
        , m_parentBranch(parent_branch)
        , m_branch_axis_type(axis_type)
        , m_branch_order(order)
        , m_radius(radius)
        , m_id(IdentifierUtility::instance().useNextPlantElementIdentifier())
    {
    }

    Branch::~Branch()
    {
    }
} /* namespace rootmap */


