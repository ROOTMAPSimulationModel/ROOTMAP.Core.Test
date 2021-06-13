#ifndef VolumeObject_H
#define VolumeObject_H

/*
 *    Abstract base class for a simple, convex, three-dimensional polyhedron object
 *  with its main axis aligned with the Z-axis.
 */

#include "simulation/process/common/Process.h"
#include "simulation/process/raytrace/RaytracerData.h"
#include "boost/geometry/geometry.hpp"
#include <list>
#include <vector>

#define NEAR_AS_INTERSECTING (1e-8)

namespace rootmap
{
    typedef boost::geometry::model::d2::point_xy<double> Pt;
    typedef boost::geometry::model::polygon<Pt> P;

    class VolumeObject : public Process
    {
    public:
        enum Surface
        {
            Top,
            Bottom,
            Left,
            Right,
            Front,
            Back,
            Curved,
            NONE
        };

        enum InteractionType
        {
            DeflectionToOutside,
            DeflectionToInside,
            PenetrationToOutside,
            PenetrationToInside,
            NoInteraction
        };

        enum RootDeflectionProbabilityCalculationAlgorithm
        {
            DirectlySpecified,
            CosinProportional
        };

        VolumeObject() {}
        virtual ~VolumeObject() {}

        virtual void SetDrawing(ProcessDrawing* drawing) = 0;

        // Returns true IFF the parameter point is inside this VO.
        virtual const bool Contains(const DoubleCoordinate& point) const = 0;
        // Returns true IFF the parameter cross-sectional polygon is entirely contained within this VO's cross-section.
        // More detail:
            // 1. This VO determines its extrema
            // 2. This VO passes each extrema point or polygon
            //    (e.g. left front top, right back bottom, the top cross-sectional circle, the bottom cross-sectional circle)
            //    to the parameter VO
            // 3. The parameter VO tests the extrema either by comparing 3D coordinates or by using simple Z-axis comparison and 2D boost::within
            // 4. The parameter VO returns a yes or no as to whether the extrema are inside it
            // 5. If all this VO's extrema are inside the parameter VO, given that VOs are all convex polyhedra, this->Within(parameterVo) == true.
        virtual const bool Contains(const P& crossSectionalPolygon, const double& z) const = 0;
        // Returns true IFF this VO is located entirely inside the parameter VO.
        virtual const bool Within(const VolumeObject* vo) const = 0;

        // Returns a pseudorandom DoubleCoordinate located inside the parameter DoubleBox.
        // If the boolean parameter is TRUE, the DoubleCoordinate will also be located inside this VolumeObject
        // (that is, it will be located in the coincident volume, the volumetric intersection of the DoubleBox and VolumeObject).
        // If the boolean parameter is FALSE, the DoubleCoordinate will NOT be located inside this VolumeObject
        // (that is, it will be located in the volume which is the volumetric difference of the DoubleBox and VolumeObject).
        virtual const DoubleCoordinate GenerateRandomPointInBox(const DoubleBox& box, const bool& insideThisVO) = 0;

        // Returns true if an intersection between the origin-->destination line segment and this VolumeObject is found.
        // otherwise returns false.
        // If returns true, assigns the coordinates to intersection
        virtual const bool GetClosestIntersection(const DoubleCoordinate& origin, const DoubleCoordinate& destination, DoubleCoordinate& intersectionDC, Vec3d& normalToIntersectedSurface, Surface& intersectedSurface) = 0;

        virtual InteractionType DeflectRootSegment(const DoubleCoordinate& startPoint, DoubleCoordinate& intersectionPoint, DoubleCoordinate& endPoint, Vec3d& unitHeadingVector, const double& rootRadius, const long& branch_order) = 0;

        virtual const size_t& GetIndex() const = 0;

        // Returns true if this VolumeObject occupies any of the same space as the parameter DoubleBox
        virtual const double GetCoincidentProportion(const DoubleBox& box, TransferRestrictedDimensionBitset& transferRestrictedDimensions, TransferRestrictedDimensionPermeabilities& transferRestrictedDimensionPermeabilities) = 0;

        // Inherited Process methods. Non-virtual as all VolumeObjects must be drawable/renderable.
        bool DoesDrawing() const;
        bool DoesRaytracerOutput() const;

        virtual const double GetRootPenetrationProbability(const Surface& surfaceToTest, const double& angleBetweenRootAndNormal) const = 0;
        virtual const double GetPermeability(const Surface& surfaceToTest) const = 0;

        virtual const RootDeflectionProbabilityCalculationAlgorithm& GetRootDeflectionProbabilityCalculationAlgorithm() const = 0;
    };

    inline bool VolumeObject::DoesDrawing() const
    {
        return true;
    }

    inline bool VolumeObject::DoesRaytracerOutput() const
    {
        return true;
    }

} /* namespace rootmap */

#endif // #ifndef VolumeObject_H
