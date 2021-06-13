#ifndef BoundingRectangularPrism_H
#define BoundingRectangularPrism_H

//
//    MSA 10.06.29
//    Derived class of VolumeObject representing a right rectangular prism.
//    This is the simplest implementation of VolumeObject.
//    It is currently constrained to vertical alignment to simplify the maths required
//    to determine its interaction with an arbitrary point (e.g. water/nitrate/phosphorus)
//    or line segment (root).
//

#include "simulation/process/modules/VolumeObject.h"

namespace rootmap
{
    class BoundingRectangularPrism : public VolumeObject, public DoubleBox
    {
    public:
        BoundingRectangularPrism(const size_t index, const DoubleCoordinate& leftFrontTop, const DoubleCoordinate& rightBackBottom, const std::string& algo, const double& topRPP = 0.0, const double& bottomRPP = 0.0, const double& sideRPP = 0.0, const double& topP = 0.0, const double& bottomP = 0.0, const double& sideP = 0.0);
        virtual ~BoundingRectangularPrism();

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
        const DoubleCoordinate& GetLeftFrontTop() const;
        const DoubleCoordinate& GetRightBackBottom() const;
        const double& GetWidth() const;
        const double& GetHeight() const;
        const double& GetDepth() const;

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
        BoundingRectangularPrism(const BoundingRectangularPrism& /* rhs */);
        void operator=(const BoundingRectangularPrism& /* rhs */);


        double m_height, m_width, m_depth;

        double m_topRPP;
        double m_bottomRPP;
        double m_sideRPP;
        double m_topP;
        double m_bottomP;
        double m_sideP;

        RootDeflectionProbabilityCalculationAlgorithm m_deflectionAlgorithm;

        P m_crossSection;

        // The defining coordinates, plus useful derived coordinates TopDeadCentre and BottomDeadCentre
        DoubleCoordinate m_leftFrontTop, m_rightBackBottom, m_TDC, m_BDC;
        bool m_squareCrossSection;

        bool m_cachedIntersectionPointValid;
        DoubleCoordinate m_cachedIntersectionPoint;
        Vec3d m_cachedNormalToIntersectedSurface;

        bool m_raytracerDataHasBeenOutput;
        bool m_doubleIntersection;
    };

    inline const size_t& BoundingRectangularPrism::GetIndex() const
    {
        return m_index;
    }

    inline const DoubleCoordinate& BoundingRectangularPrism::GetLeftFrontTop() const
    {
        return m_leftFrontTop;
    }

    inline const DoubleCoordinate& BoundingRectangularPrism::GetRightBackBottom() const
    {
        return m_rightBackBottom;
    }

    // Width == X
    inline const double& BoundingRectangularPrism::GetWidth() const
    {
        return m_width;
    }

    // Height == Y
    inline const double& BoundingRectangularPrism::GetHeight() const
    {
        return m_height;
    }

    // Depth == Length == Z
    inline const double& BoundingRectangularPrism::GetDepth() const
    {
        return m_depth;
    }

    inline const double BoundingRectangularPrism::GetPermeability(const Surface& surfaceToTest) const
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

    inline const VolumeObject::RootDeflectionProbabilityCalculationAlgorithm& BoundingRectangularPrism::GetRootDeflectionProbabilityCalculationAlgorithm() const
    {
        return m_deflectionAlgorithm;
    }
}
#endif // #ifndef BoundingRectangularPrism_H
