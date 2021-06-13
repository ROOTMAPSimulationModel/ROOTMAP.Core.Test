#ifndef Branch_H
#define Branch_H

#include "core/common/Structures.h"
#include "core/common/DoubleCoordinates.h"
#include "simulation/common/Types.h"
#include <list>

namespace rootmap
{
    class RootTip;

    class Branch
    {
        TCL_DECLARE_CLASS

    public:
        Branch();
        Branch(DoubleCoordinate* position, const double& length, const double& start_age, RootTip* child, Branch* childBranch, Branch* parent_branch, const AxisType& axis_type, const long& order, const double& radius);

        //
        // Don't need this for deleting pointer members, but for making it virtual
        // so that the Axis destructor is called.
        //
        // The pointer members are dealt with in Plant::ForgetBranch
        virtual ~Branch();

    public:
        DoubleCoordinate m_position;
        std::list<DoubleCoordinate> m_midpoints;
        double m_length;
        double m_age;
        RootTip* m_child;
        Branch* m_childBranch;
        Branch* m_parentBranch;
        AxisType m_branch_axis_type;
        // MSA 11.04.04 New member variables, moved here for optimisation
        long m_branch_order;
        double m_radius;

        PlantElementIdentifier m_id;
    };
} /* namespace rootmap */

#endif // #ifndef Branch_H
