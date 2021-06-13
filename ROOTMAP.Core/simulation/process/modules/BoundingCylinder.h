#ifndef BoundingCylinder_H
#define BoundingCylinder_H

//
//    MSA 10.06.29
//    Derived class of VolumeObject representing a truncated bounding cylinder.
//    Specifically, a pot, PVC pipe or similar enclosure used for plantings.
//    This is a simple implementation of VolumeObject.
//    It is currently constrained to vertical alignment to simplify the maths required
//    to determine its interaction with an arbitrary point (e.g. water/nitrate/phosphorus)
//    or line segment (root).
//

#include "simulation/process/modules/VolumeObject.h"

namespace rootmap
{
    class BoundingCylinder : public VolumeObject
    {
    public:
        BoundingCylinder(const size_t index, const DoubleCoordinate& topFaceCentre, const double& height, const double& radius, const std::string& algo, const double& topRPP = 0.0, const double& bottomRPP = 0.0, const double& sideRPP = 0.0, const double& topP = 0.0, const double& bottomP = 0.0, const double& sideP = 0.0);
        virtual ~BoundingCylinder();

        // VolumeObject interface methods
        void SetDrawing(ProcessDrawing* drawing);
        const bool Contains(const DoubleCoordinate& point) const;
        const bool Contains(const P& crossSectionalPolygon, const double& z) const;
        const bool Within(const VolumeObject* vo) const;
        const DoubleCoordinate GenerateRandomPointInBox(const DoubleBox& box, const bool& insideThisVO);
        const bool GetClosestIntersection(const DoubleCoordinate& origin, const DoubleCoordinate& destination, DoubleCoordinate& intersectionDC, Vec3d& normalToIntersectedSurface, Surface& intersectedSurface);
        const double GetCoincidentProportion(const DoubleBox& box, TransferRestrictedDimensionBitset& transferRestrictedDimensions, TransferRestrictedDimensionPermeabilities& transferRestrictedDimensionPermeabilities);

        InteractionType DeflectRootSegment(const DoubleCoordinate& startPoint, DoubleCoordinate& intersectionPoint, DoubleCoordinate& endPoint, Vec3d& unitHeadingVector, const double& rootRadius, const long& branch_order);

        // Accessors
        DoubleCoordinate GetOrigin() const;
        const double& GetHeight() const;
        const double& GetRadius() const;

        void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard);
        long int DoRaytracerOutput(RaytracerData* raytracerData);

        const size_t& GetIndex() const;

        const double GetRootPenetrationProbability(const Surface& surfaceToTest, const double& angleBetweenRootAndNormal) const;
        const double GetPermeability(const Surface& surfaceToTest) const;

        const RootDeflectionProbabilityCalculationAlgorithm& GetRootDeflectionProbabilityCalculationAlgorithm() const;

    private:
        ProcessDrawing* m_drawing;
        const size_t m_index;

        // Hidden copy constructor and assignment operator declaration
        BoundingCylinder(const BoundingCylinder& /* rhs */);
        void operator=(const BoundingCylinder& /* rhs */);

        P m_crossSection;

        DoubleCoordinate m_topFaceCentre;
        double m_height;
        double m_radius;

        double m_topRPP;
        double m_bottomRPP;
        double m_sideRPP;
        double m_topP;
        double m_bottomP;
        double m_sideP;

        RootDeflectionProbabilityCalculationAlgorithm m_deflectionAlgorithm;

        bool m_inboundIntersection;
        bool m_raytracerDataHasBeenOutput;
    };

    inline const size_t& BoundingCylinder::GetIndex() const
    {
        return m_index;
    }

    inline DoubleCoordinate BoundingCylinder::GetOrigin() const
    {
        return m_topFaceCentre;
    }

    inline const double& BoundingCylinder::GetHeight() const
    {
        return m_height;
    }

    inline const double& BoundingCylinder::GetRadius() const
    {
        return m_radius;
    }

    inline const double BoundingCylinder::GetPermeability(const Surface& surfaceToTest) const
    {
        switch (surfaceToTest)
        {
        case Top:
            return m_topP;
        case Bottom:
            return m_bottomP;
        default:
            return m_sideP;
        }
    }

    inline const VolumeObject::RootDeflectionProbabilityCalculationAlgorithm& BoundingCylinder::GetRootDeflectionProbabilityCalculationAlgorithm() const
    {
        return m_deflectionAlgorithm;
    }
}
#endif // #ifndef BoundingCylinder_H
