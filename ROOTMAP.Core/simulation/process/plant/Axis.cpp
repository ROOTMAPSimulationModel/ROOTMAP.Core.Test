#include "simulation/process/plant/Axis.h"


namespace rootmap
{
    Axis::Axis(const double& start_lag,
        DoubleCoordinate* position,
        Vec3d* start_orientation,
        const AxisType& type)
        : Branch(position, 0, 0, 0, 0, 0, type, -1, -1) // MSA 11.04.04 Branch order and radius not relevant for Axes
        , m_startLag(start_lag)
        , m_startOrientation(*start_orientation)
        , m_axisType(type)
    {
    }

    Axis::~Axis()
    {
    }

    void Axis::SetNext(Axis* new_axis)
    {
        if (m_parentBranch != 0)
        {
            Axis* parentBranch = dynamic_cast<Axis *>(m_parentBranch);
            if (parentBranch != 0)
            {
                parentBranch->SetNext(new_axis);
            }
        }
        else
        {
            m_parentBranch = new_axis;
        }
    }

    Axis* Axis::RemoveNext()
    {
        Axis* oldParentBranch = dynamic_cast<Axis *>(m_parentBranch);

        if (oldParentBranch != 0)
        {
            m_parentBranch = m_parentBranch->m_parentBranch;
        }

        return oldParentBranch;
    }

    Axis* Axis::GetNext() const
    {
        return (dynamic_cast<Axis *>(m_parentBranch));
    }
} /* namespace rootmap */


