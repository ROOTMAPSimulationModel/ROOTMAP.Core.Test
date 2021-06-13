#ifndef Axis_H
#define Axis_H

#include "core/common/Structures.h"
#include "core/common/DoubleCoordinates.h"
#include "simulation/process/plant/Branch.h"

namespace rootmap
{
    /* Axis
    The Axis class is a subclass of Branch. It uses the Branch
    data members for the same purpose, except
    */

    class Axis : public Branch
    {
    public:
        Axis(const double& start_lag,
            DoubleCoordinate* position,
            Vec3d* start_orientation,
            const AxisType& type);

        virtual ~Axis();

        double GetStartLag() const;
        void GetStartOrientation(Vec3d* start_orientation);
        AxisType GetType() const;

        // Convenience functions: the "next" axis is the "m_parentBranch"
        void SetNext(Axis* next);
        Axis* RemoveNext();
        Axis* GetNext() const;

    private:
        double m_startLag;
        Vec3d m_startOrientation;
        AxisType m_axisType;
    };

    inline double Axis::GetStartLag() const
    {
        return (m_startLag);
    }

    inline void Axis::GetStartOrientation(Vec3d* start_orientation)
    {
        *start_orientation = m_startOrientation;
    }

    inline AxisType Axis::GetType() const
    {
        return (m_axisType);
    }
} /* namespace rootmap */

#endif // #ifndef Axis_H
