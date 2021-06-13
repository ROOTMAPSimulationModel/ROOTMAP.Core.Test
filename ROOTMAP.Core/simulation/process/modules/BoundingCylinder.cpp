#include "simulation/process/modules/BoundingCylinder.h"
#include "simulation/process/common/ProcessDrawing.h"

namespace rootmap
{
    using namespace boost::geometry;
    typedef std::vector<P> PV;
    typedef model::box<Pt> Box2D;

    BoundingCylinder::BoundingCylinder(const size_t index, const DoubleCoordinate& topFaceCentre, const double& height, const double& radius, const std::string& algo, const double& topRPP, const double& bottomRPP, const double& sideRPP, const double& topP, const double& bottomP, const double& sideP)
        : m_drawing(__nullptr)
        , m_index(index)
        , m_height(height)
        , m_radius(radius)
        , m_topFaceCentre(topFaceCentre)
        , m_inboundIntersection(false)
        , m_raytracerDataHasBeenOutput(false)
        , m_topRPP(topRPP)
        , m_bottomRPP(bottomRPP)
        , m_sideRPP(sideRPP)
        , m_topP(topP)
        , m_bottomP(bottomP)
        , m_sideP(sideP)
        , m_deflectionAlgorithm(CosinProportional)
    {
        if (algo == "directly specified"
            || algo == "directlyspecified"
            || algo == "directly_specified")
        {
            m_deflectionAlgorithm = DirectlySpecified;
        }
        // Otherwise, leave as the default (CosinProportional).

        // Initialise the cross-section approximation polygon
        double angle = 0;
        // N.B. if resolution is excessively high (e.g. 1000)
        // the resulting circle-approximating-polygon coordinates
        // are very close together, and Boost::geometry::intersection
        // returns incorrect results. Stick with a lower resolution to be safe.
        const int resolution = 100;
        const double increment = 2 * PI / resolution;
        std::vector<Pt> coords;
        for (int i = 0; i < resolution; ++i, angle += increment)
        {
            coords.push_back(Pt(m_topFaceCentre.x + m_radius * cos(angle), m_topFaceCentre.y + m_radius * sin(angle)));
        }
        coords.push_back(coords[0]);
        assign_points(m_crossSection, coords);
        correct(m_crossSection);
    }

    BoundingCylinder::~BoundingCylinder()
    {
    }

    void BoundingCylinder::SetDrawing(ProcessDrawing* drawing)
    {
        m_drawing = drawing;
    }

    void BoundingCylinder::DrawScoreboard(const DoubleRect& /* area */, Scoreboard* /* scoreboard */)
    {
        if (m_drawing != __nullptr)
        {
            static wxColour c(0, 255, 0, 63);
            const DoubleCoordinate dc1 = m_topFaceCentre;
            const DoubleCoordinate dc2(dc1.x, dc1.y, dc1.z + m_height);
            m_drawing->DrawCone(dc1, dc2, m_radius, m_radius, c, vNONE, 12, true);
        }
    }

    long int BoundingCylinder::DoRaytracerOutput(RaytracerData* raytracerData)
    {
        if (!m_raytracerDataHasBeenOutput && raytracerData != __nullptr)
        {
            raytracerData->AddCylinder(m_topFaceCentre, m_radius, m_height);
            m_raytracerDataHasBeenOutput = true;
        }

        return kNoError;
    }

    const bool BoundingCylinder::Contains(const DoubleCoordinate& point) const
    {
        if (point.z > m_topFaceCentre.z && point.z < m_topFaceCentre.z + m_height)
        {
            const double xx = (point.x - m_topFaceCentre.x) * (point.x - m_topFaceCentre.x);
            const double yy = (point.y - m_topFaceCentre.y) * (point.y - m_topFaceCentre.y);
            if (xx + yy < m_radius * m_radius)
            {
                return true;
            }
        }
        return false;
    }

    const bool BoundingCylinder::Contains(const P& crossSectionalPolygon, const double& z) const
    {
        if (z > m_topFaceCentre.z && z < m_topFaceCentre.z + m_height)
        {
            return within(crossSectionalPolygon, m_crossSection);
        }
        return false;
    }

    const bool BoundingCylinder::Within(const VolumeObject* vo) const
    {
        return vo != __nullptr
            && vo->Contains(m_crossSection, m_topFaceCentre.z)
            && vo->Contains(m_crossSection, m_topFaceCentre.z + m_height);
    }

    // MSA 11.02.16 Trying a new "jitter-lock" approach for simplicity/efficiency
    const DoubleCoordinate BoundingCylinder::GenerateRandomPointInBox(const DoubleBox& box, const bool& insideThisVO)
    {
        DoubleCoordinate dc;

        // Start by treating this Cylinder as a rectangular prism for simplicity.

        const double voPseudoLeft = m_topFaceCentre.x - m_radius;
        const double voPseudoRight = m_topFaceCentre.x + m_radius;
        const double voPseudoFront = m_topFaceCentre.y - m_radius;
        const double voPseudoBack = m_topFaceCentre.y + m_radius;
        const double voTop = m_topFaceCentre.z;
        const double voBottom = m_topFaceCentre.z + m_height;

        bool useJitterLock = false;

        if (insideThisVO)
        {
            const double xmin = Utility::CSMax(voPseudoLeft, box.left);
            const double xmax = Utility::CSMin(voPseudoRight, box.right);
            const double ymin = Utility::CSMax(voPseudoFront, box.front);
            const double ymax = Utility::CSMin(voPseudoBack, box.back);
            const double zmin = Utility::CSMax(voTop, box.top);
            const double zmax = Utility::CSMin(voBottom, box.bottom);

            RmAssert(box.top<voBottom && box.bottom>voTop, "Volumes are not at all coincident in Z plane");

            size_t iterations = 0;
            do
            {
                dc.x = Utility::RandomRange(xmax, xmin);
                dc.y = Utility::RandomRange(ymax, ymin);
                dc.z = Utility::RandomRange(zmax, zmin);
                ++iterations;
                if (iterations == 100)
                {
                    TransferRestrictedDimensionBitset bs;
                    TransferRestrictedDimensionPermeabilities p;
                    const double cp = GetCoincidentProportion(box, bs, p);
                    if (cp <= 0.00001)
                    {
                        useJitterLock = true;
                        break;
                    }
                    // else it's just bad luck, or something... keep going
                }
                else if (iterations == 1000)
                {
                    useJitterLock = true;
                    break;
                }
            } while (!this->Contains(dc));


            if (useJitterLock)
            {
                while (!(dc.z > voTop && dc.z < voBottom))
                {
                    dc.z = Utility::RandomRange(box.bottom, box.top);
                }
                bool xyOK = false;
                do
                {
                    dc.x = Utility::RandomRange(xmax, xmin);
                    dc.y = Utility::RandomRange(ymax, ymin);

                    const double xx = (dc.x - m_topFaceCentre.x) * (dc.x - m_topFaceCentre.x);
                    const double yy = (dc.y - m_topFaceCentre.y) * (dc.y - m_topFaceCentre.y);
                    xyOK = (xx + yy < m_radius * m_radius);
                } while (!xyOK);
            }
        }
        else
        {
            const double xmin = box.left;
            const double xmax = box.right;
            const double ymin = box.front;
            const double ymax = box.back;
            const double zmin = box.top;
            const double zmax = box.bottom;

            size_t iterations = 0;
            do
            {
                dc.x = Utility::RandomRange(xmax, xmin);
                dc.y = Utility::RandomRange(ymax, ymin);
                dc.z = Utility::RandomRange(zmax, zmin);
                ++iterations;
                if (iterations == 100)
                {
                    TransferRestrictedDimensionBitset bs;
                    TransferRestrictedDimensionPermeabilities p;
                    const double cp = GetCoincidentProportion(box, bs, p);
                    if (cp > 0.9999)
                    {
                        useJitterLock = true;
                        break;
                    }
                    // else it's just bad luck, or something... keep going
                }
                else if (iterations == 1000)
                {
                    useJitterLock = true;
                    break;
                }
            } while (this->Contains(dc));

            if (useJitterLock)
            {
                const bool zOverlap = !((box.bottom <= voTop) || (box.top >= voBottom));
                const bool totalZOverlap = zOverlap && box.bottom <= voBottom && box.top >= voTop;

                while (zOverlap && !totalZOverlap && dc.z >= voTop && dc.z <= voBottom)
                {
                    dc.z = Utility::RandomRange(box.bottom, box.top);
                }
                bool xyOK = !zOverlap || (zOverlap && !totalZOverlap);
                while (!xyOK)
                {
                    dc.x = Utility::RandomRange(xmax, xmin);
                    dc.y = Utility::RandomRange(ymax, ymin);

                    const double xx = (dc.x - m_topFaceCentre.x) * (dc.x - m_topFaceCentre.x);
                    const double yy = (dc.y - m_topFaceCentre.y) * (dc.y - m_topFaceCentre.y);
                    xyOK = (xx + yy > m_radius * m_radius);
                }
            }
        }

        return dc;
    }

    const double BoundingCylinder::GetCoincidentProportion(const DoubleBox& box, TransferRestrictedDimensionBitset& transferRestrictedDimensions, TransferRestrictedDimensionPermeabilities& transferRestrictedDimensionPermeabilities)
    {
        const Pt rightBack(box.right, box.back);
        const Pt leftBack(box.left, box.back);
        const Pt leftFront(box.left, box.front);
        const Pt rightFront(box.right, box.front);

        // MSA TODO Are these gradients always 0 and [infinity] by definition? No. Boxes are arbitrarily defined.
        const double slopeL = (leftBack.get<1>() - leftFront.get<1>()) / (leftBack.get<0>() - leftFront.get<0>());
        const double slopeR = (rightBack.get<1>() - rightFront.get<1>()) / (rightBack.get<0>() - rightFront.get<0>());
        const double slopeF = (rightFront.get<1>() - leftFront.get<1>()) / (rightFront.get<0>() - leftFront.get<0>());
        const double slopeB = (rightBack.get<1>() - leftBack.get<1>()) / (rightBack.get<0>() - leftBack.get<0>());

        /*    Box2D cb(leftFront, rightBack);
            correct(cb);*/
        P polybox;
        append(polybox, rightBack);
        append(polybox, rightFront);
        append(polybox, leftFront);
        append(polybox, leftBack);
        append(polybox, rightBack);
        correct(polybox);

        PV intersectionPolygons;
        boost::geometry::intersection(polybox, m_crossSection, intersectionPolygons);
        /*  PV intersectionPolygons2;
          boost::geometry::intersection(polybox, m_crossSection, intersectionPolygons2);*/

        const double boxVolume = area(polybox) * (box.bottom - box.top);
        double coincidentProportion = 0;
        model::linestring<Pt> polygonOutline;
        bool addXPos = false, addXNeg = false, addYPos = false, addYNeg = false, addZPos = false, addZNeg = false;

        bool leftIntersected = false, leftMatched = false, rightIntersected = false, rightMatched = false;
        bool frontIntersected = false, frontMatched = false, backIntersected = false, backMatched = false;

        // Note that because the intersection is m_crossSection-with-rectangle, there should only be 0 or 1 polygons in this PV.
        for (PV::iterator it = intersectionPolygons.begin(); it != intersectionPolygons.end(); ++it)
        {
            const bool completeXYOverlap = equals(*it, polybox);
            const double thisXYArea = area(*it);

            if (thisXYArea > 0)
            {
                if (!completeXYOverlap)
                {
                    model::linestring<Pt> polygonLines(it->outer().begin(), it->outer().end());
                    // Clip the bounding-box linestring with this polygon
                    // Some kind of generic-programming nightmare happens here using this...
                    //model::linestring<Pt> ls;
                    //ls.push_back(leftBack);
                    //ls.push_back(rightBack);
                    //std::vector<model::linestring<Pt>> clipped;
                    //intersection_inserter<model::linestring<Pt>> (polygonLines, ls, std::back_inserter(clipped));
                    // ...so we do it the old fashioned way.

                    model::linestring<Pt>::const_iterator iter = polygonLines.begin();
                    Pt p1 = *iter;
                    ++iter; // This code assumes at least 2 valid points in the linestring. There logically must be at least 3 points, as thisArea>0.
                    do
                    {
                        Pt p2 = *(iter);

                        const double rise = (p2.get<1>() - p1.get<1>());
                        const double run = (p2.get<0>() - p1.get<0>());

                        //Vec3d ab(run,rise,0);    // Vector from p1 to p2

                        // This is to ensure -infinity is treated the same as infinity
                        const double slope = ((run == 0 && rise < 0) ? -1 : 1) * rise / run;

                        if (slope == slopeL) // ab is parallel to left side of box
                        {
                            Vec3d ab(leftFront.get<0>() - p1.get<0>(), leftFront.get<1>() - p1.get<1>(), 0); // Vector from p1 to leftFront
                            Vec3d ac(leftBack.get<0>() - p1.get<0>(), leftBack.get<1>() - p1.get<1>(), 0); // Vector from p1 to leftBack

                            if ((ab.x * ac.y) - (ab.y * ac.x) == 0) // ab and ac are colinear
                            {
                                // If we are here, ab must be a scalar (in the range (0,1]) multiple of the left side of the box.
                                leftIntersected = true;
                                if (equals(p1, leftBack) && equals(p2, leftFront)
                                    || equals(p1, leftFront) && equals(p2, leftBack))
                                {
                                    // If we are here, ab == left side.
                                    leftMatched = true;
                                }
                            }
                        }

                        if (slope == slopeR) // ab is parallel to right side of box
                        {
                            Vec3d ab(rightFront.get<0>() - p1.get<0>(), rightFront.get<1>() - p1.get<1>(), 0); // Vector from p1 to rightFront
                            Vec3d ac(rightBack.get<0>() - p1.get<0>(), rightBack.get<1>() - p1.get<1>(), 0); // Vector from p1 to rightBack

                            if ((ab.x * ac.y) - (ab.y * ac.x) == 0) // ab and ac are colinear
                            {
                                // If we are here, ab must be a scalar (in the range (0,1]) multiple of the right side of the box.
                                rightIntersected = true;
                                if (equals(p1, rightBack) && equals(p2, rightFront)
                                    || equals(p1, rightFront) && equals(p2, rightBack))
                                {
                                    // If we are here, ab == right side.
                                    rightMatched = true;
                                }
                            }
                        }

                        if (slope == slopeF) // ab is parallel to front side of box
                        {
                            Vec3d ab(leftFront.get<0>() - p1.get<0>(), leftFront.get<1>() - p1.get<1>(), 0); // Vector from p1 to leftFront
                            Vec3d ac(rightFront.get<0>() - p1.get<0>(), rightFront.get<1>() - p1.get<1>(), 0); // Vector from p1 to rightFront

                            if ((ab.x * ac.y) - (ab.y * ac.x) == 0) // ab and ac are colinear
                            {
                                // If we are here, ab must be a scalar (in the range (0,1]) multiple of the front side of the box.
                                frontIntersected = true;
                                if (equals(p1, leftFront) && equals(p2, rightFront)
                                    || equals(p1, rightFront) && equals(p2, leftFront))
                                {
                                    // If we are here, ab == front side.
                                    frontMatched = true;
                                }
                            }
                        }

                        if (slope == slopeB) // ab is parallel to back side of box
                        {
                            Vec3d ab(leftBack.get<0>() - p1.get<0>(), leftBack.get<1>() - p1.get<1>(), 0); // Vector from p1 to leftBack
                            Vec3d ac(rightBack.get<0>() - p1.get<0>(), rightBack.get<1>() - p1.get<1>(), 0); // Vector from p1 to rightBack

                            if ((ab.x * ac.y) - (ab.y * ac.x) == 0) // ab and ac are colinear
                            {
                                // If we are here, ab must be a scalar (in the range (0,1]) multiple of the back side of the box.
                                backIntersected = true;
                                if (equals(p1, leftBack) && equals(p2, rightBack)
                                    || equals(p1, rightBack) && equals(p2, leftBack))
                                {
                                    // If we are here, ab == back side.
                                    backMatched = true;
                                }
                            }
                        }

                        p1 = p2;
                    } while (++iter != polygonLines.end());
                } // else (i.e. if completeXYOverlap): Box's XY cross-section is entirely contained within this BoundingCylinder's XY cross-section.


                // Now, because this is a cylinder, if there is complete XY-plane area overlap, then none of the X or Y faces are obstructed, by definition.
                // The curved edge of the BoundingCylinder cannot be coplanar with the flat box faces.

                double coincidentZProportion = 0.0;
                // This conditional evaluates to true if the box intersects the top of the BoundingCylinder OR the boxtop or boxbottom is coplanar with same
                if (box.top <= m_topFaceCentre.z)
                {
                    if (box.bottom == m_topFaceCentre.z)
                    {
                        // When the box is "sitting on top of" the VO, all of the box is transferable upwards.
                        addZNeg = false;
                        // If and only if there is less than complete overlap, all of the box is transferable downwards 
                        // (because everything in the box can, theoretically, get through the noncoincident section of the coplanar surfaces).
                        if (completeXYOverlap) addZPos = true;

                        coincidentZProportion = 0.0;
                    }
                    else if (box.bottom > m_topFaceCentre.z)
                    {
                        if (completeXYOverlap)
                        {
                            // If there is complete overlap between the XY cross-section of the BoundingCylinder and this box, then both Z-directions are obstructed.
                            addZNeg = true;
                            addZPos = true;
                        }
                        else
                        {
                            // Otherwise, the Scoreboard box face with partial coincidence with the VO is not obstructed 
                            // (because it can "access" both the VO-coincident and -noncoincident subsections)
                            // but the face without the partial coincidence IS obstructed.
                            addZNeg = true;
                            addZPos = false;
                        }
                        // Determine the proportion of volume which is coincident:
                        coincidentZProportion = (box.bottom - m_topFaceCentre.z) / (box.bottom - box.top);
                    }
                }
                // This conditional evaluates to true if the box intersects the bottom of the BoundingCylinder OR the boxtop or boxbottom is coplanar with same
                if (box.bottom >= (m_topFaceCentre.z + m_height))
                {
                    if (box.top == (m_topFaceCentre.z + m_height))
                    {
                        // If there is complete overlap between the XY cross-section of the BoundingCylinder and this box, then the Z-direction is obstructed.
                        // When the VO is "sitting on top of" the box, all of the box is transferable downwards.
                        addZPos = false;
                        // If and only if there is less than complete overlap, all of the box is transferable upwards 
                        // (because everything in the box can, theoretically, get through the noncoincident section of the coplanar surfaces).
                        if (completeXYOverlap) addZNeg = true;

                        coincidentZProportion = 0.0;
                    }
                    else if (box.top <= (m_topFaceCentre.z + m_height))
                    {
                        if (completeXYOverlap)
                        {
                            // If there is complete overlap between the XY cross-section of the BoundingCylinder and this box, then both Z-directions are obstructed.
                            addZNeg = true;
                            addZPos = true;
                        }
                        else
                        {
                            // Otherwise, the Scoreboard box face with partial coincidence with the VO is not obstructed 
                            // (because it can "access" both the VO-coincident and -noncoincident subsections)
                            // but the face without the partial coincidence IS obstructed.
                            addZNeg = false;
                            addZPos = true;
                        }
                        // Determine the proportion of volume which is coincident:
                        coincidentZProportion = ((m_topFaceCentre.z + m_height) - box.top) / (box.bottom - box.top);
                    }
                }
                // Is the box entirely within the Cylinder's Z-range?
                if (box.top >= m_topFaceCentre.z && box.bottom <= (m_topFaceCentre.z + m_height))
                {
                    coincidentZProportion = 1.0;
                }


                // NOTE: as this is a cylinder, axially aligned with the scoreboard, the cross-section is the same for the top and the bottom of the box.


                // Now, we evaluate the transfer restriction applicable to each face of this Box due to interaction with this BoundingCylinder.

                // Asterisk refers to this note:
                // * (and whatever will permeate through the VO surface)

                // X-wise:
                if (leftIntersected && !rightIntersected) addXPos = true; // VO intersects with the left side only; transfer out rightwards is restricted to the noncoincident section*
                if (rightIntersected && !leftIntersected) addXNeg = true; // VO intersects with the right side only; transfer out leftwards is restricted to the noncoincident section*
                if (coincidentZProportion == 1.0)
                {
                    if (leftMatched && ((rightIntersected && !rightMatched) || (!rightIntersected))) addXNeg = true; // VO blocks whole left side but only part or none of right side; transfer out leftwards is restricted to the coincident section*
                    if (rightMatched && ((leftIntersected && !leftMatched) || (!leftIntersected))) addXPos = true; // VO blocks whole right side but only part or none of left side; transfer out rightwards is restricted to the coincident section*
                }
                // Y-wise:
                if (frontIntersected && !backIntersected) addYPos = true; // VO intersects with the front side only; transfer out backwards is restricted to the noncoincident section*
                if (backIntersected && !frontIntersected) addYNeg = true; // VO intersects with the back side only; transfer out frontwards is restricted to the noncoincident section*
                if (coincidentZProportion == 1.0)
                {
                    if (frontMatched && ((backIntersected && !backMatched) || (!backIntersected))) addYNeg = true; // VO blocks whole front side but only part or none of back side; transfer out frontwards is restricted to the coincident section*
                    if (backMatched && ((frontIntersected && !frontMatched) || (!frontIntersected))) addYPos = true; // VO blocks whole back side but only part or none of front side; transfer out backwards is restricted to the coincident section*
                }

                coincidentProportion += thisXYArea * coincidentZProportion * (box.bottom - box.top);
            } // end if(thisArea>0)
        }

        if (addXNeg)
        {
            transferRestrictedDimensions.set(0);
            if (m_sideP > 0.0) transferRestrictedDimensionPermeabilities[XNegative] = m_sideP; // Transfer leftwards out the left
        }
        if (addXPos)
        {
            transferRestrictedDimensions.set(1);
            if (m_sideP > 0.0) transferRestrictedDimensionPermeabilities[XPositive] = m_sideP; // Transfer rightwards out the right
        }
        if (addYNeg)
        {
            transferRestrictedDimensions.set(2);
            if (m_sideP > 0.0) transferRestrictedDimensionPermeabilities[YNegative] = m_sideP; // Transfer forwards out the front
        }
        if (addYPos)
        {
            transferRestrictedDimensions.set(3);
            if (m_sideP > 0.0) transferRestrictedDimensionPermeabilities[YPositive] = m_sideP; // Transfer backwards out the back
        }
        if (addZNeg)
        {
            transferRestrictedDimensions.set(4);
            if (m_topP > 0.0) transferRestrictedDimensionPermeabilities[ZNegative] = m_topP; // Transfer upwards out the top
        }
        if (addZPos)
        {
            transferRestrictedDimensions.set(5);
            if (m_bottomP > 0.0) transferRestrictedDimensionPermeabilities[ZPositive] = m_bottomP; // Transfer downwards out the bottom
        }

        RmAssert(coincidentProportion <= boxVolume, "Logic error: TARDIS-type inner/outer spatial mismatch");

        return coincidentProportion / boxVolume;
    }

    // Returns the closest intersection, if any, between the line segment origin-->destination and this BoundingCylinder.
    const bool BoundingCylinder::GetClosestIntersection(const DoubleCoordinate& origin, const DoubleCoordinate& destination, DoubleCoordinate& intersectionDC, Vec3d& normalToIntersectedSurface, Surface& intersectedSurface)
    {
        m_inboundIntersection = false;
        if (DistanceSquared3D(&origin, &destination) < NEAR_AS_INTERSECTING * NEAR_AS_INTERSECTING) // Use the Squared variation to avoid square root function overhead
        {
            return false;
        }

        // Start by checking for intersection with the curved cylinder wall.

        // Convert origin and destination x and y to coordinate system relative to m_crossSection centre.
        const double x1 = origin.x - m_topFaceCentre.x;
        const double y1 = origin.y - m_topFaceCentre.y;
        const double x2 = destination.x - m_topFaceCentre.x;
        const double y2 = destination.y - m_topFaceCentre.y;

        // Determine effective radius: are we (originally) outside or inside this VO?
        const bool outside = origin.z < m_topFaceCentre.z || origin.z >(m_topFaceCentre.z + m_height) || (x1 * x1 + y1 * y1 > m_radius * m_radius);

        const double dx = x2 - x1;
        const double dy = y2 - y1;
        const double drSquared = dx * dx + dy * dy;

        const double determinant = x1 * y2 - y1 * x2;

        const double discriminant = m_radius * m_radius * drSquared - determinant * determinant;

        double curvedT = 1e9; // Parameter variable for parametric equations
        bool curvedSurfaceIntersected = false;

        if (discriminant <= 0)
        {
            if (discriminant == 0 && drSquared != 0)
            {
                // Tangent line.
                curvedSurfaceIntersected = true;
                intersectionDC.x = (determinant * dy) / drSquared;
                intersectionDC.y = (-determinant * dx) / drSquared;
                curvedT = (intersectionDC.x - origin.x) / dx;
                intersectedSurface = Curved;
            }
            // else if discriminant==0 then dx=dy=0; i.e. line is perfectly vertical.
            // else if discriminant<0 then no intersection.
        }
        else
        {
            const double sign = dy < 0 ? -1 : 1;

            const double partialXEqn = sign * dx * sqrt(discriminant);

            const double xPos = (determinant * dy + partialXEqn) / drSquared;
            const double xNeg = (determinant * dy - partialXEqn) / drSquared;

            const double partialYEqn = abs(dy) * sqrt(discriminant);

            const double yPos = (-determinant * dx + partialYEqn) / drSquared;
            const double yNeg = (-determinant * dx - partialYEqn) / drSquared;

            // Substitute these (x,y) values into the parametric form to determine curvedT.
            // Remember we want curvedT in the range [0,1).
            // If curvedT==0, intersection is at origin. Most likely, this is because a root segment has been projected onto the surface.
            // If curvedT==0, intersection is at destination and we do not wish to return it.
            // xPos = x1 + (x2-x1)curvedT
            // curvedT = (xPos-x1)/(x2-x1)
            curvedT = (xPos - x1) / (x2 - x1);

            if (curvedT > 0 && curvedT <= 1.0)
            {
                curvedSurfaceIntersected = true;
                intersectionDC.x = xPos + m_topFaceCentre.x;
                intersectionDC.y = yPos + m_topFaceCentre.y;
                intersectedSurface = Curved;
            }
            else
            {
                curvedT = (xNeg - x1) / (x2 - x1);
                if (curvedT > 0 && curvedT <= 1.0)
                {
                    curvedSurfaceIntersected = true;
                    intersectionDC.x = xNeg + m_topFaceCentre.x;
                    intersectionDC.y = yNeg + m_topFaceCentre.y;
                    intersectedSurface = Curved;
                }
            }
        }

        // Otherwise, check for intersection with top or bottom disc.

        // Obtain intersection of line and plane using parametric matrix algebra method
        // See: http://en.wikipedia.org/wiki/Line-plane_intersection

        // Choose simple arbitrary (but non-colinear) points on the plane: (0,0,m_topFaceCentre.z), (1,0,m_topFaceCentre.z) and (0,1,m_topFaceCentre.z)
        // [or] (0,0,m_topFaceCentre.z+m_height), (1,0,m_topFaceCentre.z+m_height) and (0,1,m_topFaceCentre.z+m_height).
        // This matrix is valid for all parallel planes where Z=k, so we can use it for the top and bottom disc-containing planes.

        bool discIntersected = false;

        const double det = origin.z - destination.z; // from simplifying the algebraic form, determinant shakes out to be za-zb.
        // Therefore, if line segment is in the XY plane - parallel to the top and bottom discs - the determinant is 0 and the discs will not be intersected.

        double discT = 1e9; // Parameter variable for parametric equations (same equations; just a different VALUE of t)
        if (det != 0)
        {
            // No need to define the original matrix - inverse has been calculated manually and tested.
            double inv[3][3];
            inv[0][0] = 0;
            inv[0][1] = 1;
            inv[0][2] = 0;
            inv[1][0] = 0;
            inv[1][1] = 0;
            inv[1][2] = 1;
            inv[2][0] = 1.0 / det;
            inv[2][1] = (destination.x - origin.x) / det;
            inv[2][2] = (destination.y - origin.y) / det;

            const double vectorToTopPlane[3] = { origin.x, // line xa - plane x0
                origin.y, // line ya - plane y0
                origin.z - m_topFaceCentre.z }; // line za - plane z0
            //double intersect[3];

            discT = /* intersect[0] = */ inv[0][0] * vectorToTopPlane[0] + inv[1][0] * vectorToTopPlane[1] + inv[2][0] * vectorToTopPlane[2];
            // Don't actually need the u and v components of intersect=={{curvedT},{u},{v}}
            //intersect[1] = inv[0][1]*vectorToTopPlane[0] + inv[1][1]*vectorToTopPlane[1] + inv[2][1]*vectorToTopPlane[2];
            //intersect[2] = inv[0][2]*vectorToTopPlane[0] + inv[1][2]*vectorToTopPlane[1] + inv[2][2]*vectorToTopPlane[2];  
            normalToIntersectedSurface.x = 0;
            normalToIntersectedSurface.y = 0;
            normalToIntersectedSurface.z = origin.z < m_topFaceCentre.z ? -1 : 1;

            if (discT > 0 && discT <= 1 && (origin.z >= m_topFaceCentre.z ? destination.z < origin.z : origin.z < destination.z))
            {
                // Root is attempting to cross top disc
                discIntersected = true;
                intersectedSurface = Top;
            }
            else discT = 1e9;

            const double vectorToBottomPlane[3] = { origin.x, // line xa - plane x0
                origin.y, // line ya - plane y0
                origin.z - (m_topFaceCentre.z + m_height) }; // line za - plane z0

            const double bottomDiscT = /* intersect[0] = */ inv[0][0] * vectorToBottomPlane[0] + inv[1][0] * vectorToBottomPlane[1] + inv[2][0] * vectorToBottomPlane[2];
            // Don't actually need the u and v components of intersect=={{curvedT},{u},{v}}
            //intersect[1] = inv[0][1]*vectorToBottomPlane[0] + inv[1][1]*vectorToBottomPlane[1] + inv[2][1]*vectorToBottomPlane[2];
            //intersect[2] = inv[0][2]*vectorToBottomPlane[0] + inv[1][2]*vectorToBottomPlane[1] + inv[2][2]*vectorToBottomPlane[2]; 

            if (bottomDiscT > 0 && bottomDiscT <= 1 && bottomDiscT < discT && (origin.z <= (m_topFaceCentre.z + m_height) ? destination.z > origin.z : origin.z > destination.z))
            {
                // Root is attempting to cross bottom disc (and if it crosses the top disc too, it's crossing the bottom first)
                discT = bottomDiscT;
                normalToIntersectedSurface.z = origin.z > (m_topFaceCentre.z + m_height) ? 1 : -1;
                discIntersected = true;
                intersectedSurface = Bottom;
            }
        }

        // Now, determine which intersection occurs first (if 2 intersections)

        if (curvedSurfaceIntersected && (!discIntersected || (discIntersected && curvedT <= (discT + NEAR_AS_INTERSECTING))))
        {
            // Find z from line eqn. Ensure within cylinder bounds.
            intersectionDC.z = origin.z + (destination.z - origin.z) * curvedT;

            if (intersectionDC.z >= m_topFaceCentre.z && intersectionDC.z <= (m_topFaceCentre.z + m_height))
            {
                normalToIntersectedSurface.x = m_topFaceCentre.x - intersectionDC.x;
                normalToIntersectedSurface.y = m_topFaceCentre.y - intersectionDC.y;
                if (outside)
                {
                    // If intersection is from the outside, the normal is parallel to the radius vector, but points outward.
                    normalToIntersectedSurface.x *= -1;
                    normalToIntersectedSurface.y *= -1;
                }
                // If intersecting two faces at essentially the same spot, put a bit of a kludge in here to bounce the roots better
                if (discIntersected && (discT - curvedT) <= NEAR_AS_INTERSECTING)
                {
                    normalToIntersectedSurface.z *= asin(0.25 * PI); // Normal at 45 degrees to XY plane
                }
                else
                {
                    normalToIntersectedSurface.z = 0;
                }
                normalToIntersectedSurface.Normalize();
                m_inboundIntersection = outside;
                return true;
            }
        }
        else if (discIntersected)
        {
            // Intersection with disc plane is between origin and destination...
            // Ensure that point on plane is inside the cone's radius.
            // i.e. substitute value of curvedT (curvedT==intersect[0]) into the line equation
            intersectionDC.x = origin.x + (destination.x - origin.x) * discT;
            intersectionDC.y = origin.y + (destination.y - origin.y) * discT;
            intersectionDC.z = origin.z + (destination.z - origin.z) * discT;
            const double ix = intersectionDC.x - m_topFaceCentre.x;
            const double iy = intersectionDC.y - m_topFaceCentre.y;

            if ((ix * ix + iy * iy) <= (m_radius * m_radius))
            {
                m_inboundIntersection = outside;
                return true;
            }
            // Finally: if the plane intersection was outside the disc, AND
            // the curved surface was intersected AFTER the plane, we need to recheck it.
            else if (curvedSurfaceIntersected)
            {
                // Find z from line eqn. Ensure within cylinder bounds.
                intersectionDC.z = origin.z + (destination.z - origin.z) * curvedT;

                if (intersectionDC.z >= m_topFaceCentre.z && intersectionDC.z <= (m_topFaceCentre.z + m_height))
                {
                    normalToIntersectedSurface.x = m_topFaceCentre.x - intersectionDC.x;
                    normalToIntersectedSurface.y = m_topFaceCentre.y - intersectionDC.y;
                    if (outside)
                    {
                        // If intersection is from the outside, the normal is parallel to the radius vector, but points outward.
                        normalToIntersectedSurface.x *= -1;
                        normalToIntersectedSurface.y *= -1;
                    }
                    // If intersecting two faces at essentially the same spot, put a bit of a kludge in here to bounce the roots better
                    if (discIntersected && (discT - curvedT) <= NEAR_AS_INTERSECTING)
                    {
                        normalToIntersectedSurface.z *= asin(0.25 * PI); // Normal at 45 degrees to XY plane
                    }
                    else
                    {
                        normalToIntersectedSurface.z = 0;
                    }
                    normalToIntersectedSurface.Normalize();
                    m_inboundIntersection = outside;
                    return true;
                }
            }
        }

        return false;
    }

    VolumeObject::InteractionType BoundingCylinder::DeflectRootSegment(const DoubleCoordinate& startPoint, DoubleCoordinate& intersectionPoint, DoubleCoordinate& endPoint, Vec3d& unitHeadingVector, const double& rootRadius, const long& /*branch_order*/)
    {
        Vec3d normalToIntersectedSurface;
        Surface intersectedSurface = NONE;

        if (!GetClosestIntersection(startPoint, endPoint, intersectionPoint, normalToIntersectedSurface, intersectedSurface))
        {
            // This root segment did not encounter any surfaces.
            return NoInteraction;
        }

        // Start by determining deflection direction.
        const double angleToNormal = unitHeadingVector.angleBetween(normalToIntersectedSurface);

        // So an intersection has been found. Can the root penetrate the surface at this intersection?
        double p = GetRootPenetrationProbability(intersectedSurface, angleToNormal);
        if (p == 1.0 || p > Utility::Random1())
        {
            // Root has penetrated and there's no deflection to be done. Thus the heading is unchanged.
            if (Contains(startPoint)) return PenetrationToOutside;
            // else
            return PenetrationToInside;
        }

        // If not, then the root is deflected.

        // Push the intersection point away from the intersected surface by the radius of the root.
        intersectionPoint = intersectionPoint + rootRadius * normalToIntersectedSurface;

        const double magnitudeAfterIntersection = Distance3D(&intersectionPoint, &endPoint);
        DoubleCoordinate deflectedEndPoint(-999, -999, -999);


        // Now, determine whether the surface is coincident with the tangent plane
        // beyond the point of intersection (i.e. top or bottom disc) or not (i.e. curved cylinder surface)
        if (normalToIntersectedSurface.x == 0 && normalToIntersectedSurface.y == 0)
        {
            // Root has intersected the top or bottom disc of the cylinder.
            // NOTE: behaviour in this case is the same regardless of inbound/outbound intersection and branch order;
            // in all cases the root is deflected along the tangent, because the tangent IS the surface.
            normalToIntersectedSurface.Normalize();
            if (normalToIntersectedSurface.dotProd(unitHeadingVector) <= 0)
            {
                // Root segment vector is inbound to surface; zero out the Z-component (as surface is a flat disc in the XY plane)
                unitHeadingVector.z = 0;
                unitHeadingVector.Normalize();
            }
            deflectedEndPoint = intersectionPoint + magnitudeAfterIntersection * unitHeadingVector;
        }
        else
        {
            if (m_inboundIntersection)// && branch_order==0)
            {
                // Root has intersected the curved surface from outside;        // it is also a primary root. NOTE: All roots now deflect along tangent.
                // Project the tangential component onto the tangent plane.
                const double defX = endPoint.x - intersectionPoint.x;
                const double defY = endPoint.y - intersectionPoint.y;
                const double deflectionMagnitude = sqrt(defX * defX + defY * defY);
                deflectedEndPoint = endPoint + deflectionMagnitude * normalToIntersectedSurface;
                unitHeadingVector = Vec3d(deflectedEndPoint.x - intersectionPoint.x,
                    deflectedEndPoint.y - intersectionPoint.y,
                    deflectedEndPoint.z - intersectionPoint.z);
                unitHeadingVector.Normalize();
            }
            else
            {
                // Root has intersected the curved surface from inside

                // Get the component of the vector in the XY plane...
                Vec3d vecXY(unitHeadingVector);
                vecXY *= magnitudeAfterIntersection;
                vecXY.z = 0;
                const double magXY = vecXY.Length();
                //RmAssert(angleToNormal>=0.5*PI, "Error");
                // and project it onto the [line of intersection with the] tangent plane
                const double tangentialComponent = magXY * cos(angleToNormal - 0.5 * PI);

                // Project the tangential component length onto the radius.

                const double arcAngle = tangentialComponent / m_radius;

                // Determine the deflection direction (counterclockwise or clockwise) which deflects the vector the least.
                // Deflection should be as minor as possible; check the angles between the undeflected vector and both potentials, select the one with the least deflection.

                const double iradialx = intersectionPoint.x - m_topFaceCentre.x;
                const double iradialy = intersectionPoint.y - m_topFaceCentre.y;
                const double uradialx = endPoint.x - m_topFaceCentre.x;
                const double uradialy = endPoint.y - m_topFaceCentre.y;

                // Determine polar coordinate angles for:
                // 1) undeflected endpoint
                // 2) intersection point
                double uPolarAngle = atan2(uradialy, uradialx); // Angle from the X=0 origin line (on the unit m_crossSection) to undeflectedEndPoint
                double iPolarAngle = atan2(iradialy, iradialx); // Angle from the X=0 origin line (on the unit m_crossSection) to intersectionPoint
                bool counterClockwise;
                if (uPolarAngle >= 0 == iPolarAngle >= 0)
                {
                    // Both in either (top two) or (bottom two) quadrants
                    counterClockwise = uPolarAngle >= iPolarAngle;
                }
                else
                {
                    // One in the top two quadrants, one in the bottom two
                    // If uPolarAngle is in 1st quadrant and iPolarAngle in 4th, we want to go CCW.
                    if (uPolarAngle >= 0 && uPolarAngle < (0.5 * PI) && iPolarAngle >(-0.5 * PI)) counterClockwise = true;
                    // If uPolarAngle is in 3rd quadrant and iPolarAngle in 2nd, we want to go CCW.
                    else if (uPolarAngle < (-0.5 * PI) && iPolarAngle >(0.5 * PI)) counterClockwise = true;
                    // In all other configurations possible here, we want to go CW.
                    else counterClockwise = false;
                }

                const double arcAngleInCorrectDirection = counterClockwise ? arcAngle : -arcAngle;

                // Use a slightly-larger or slightly-smaller radius to determine the deflected endpoint.
                const double effectiveRadius = m_inboundIntersection ? m_radius + rootRadius : m_radius - rootRadius;

                deflectedEndPoint.x = m_topFaceCentre.x + effectiveRadius * cos(iPolarAngle + arcAngleInCorrectDirection);
                deflectedEndPoint.y = m_topFaceCentre.y + effectiveRadius * sin(iPolarAngle + arcAngleInCorrectDirection);
                deflectedEndPoint.z = intersectionPoint.z + magnitudeAfterIntersection * unitHeadingVector.z; // Z-growth is unaffected by deflection.

                // Calculate the (tangential to XY-plane m_crossSection) heading and assign.
                // Remember to ensure that it's in the correct direction.
                const double z = unitHeadingVector.z;
                double x = (deflectedEndPoint.y - m_topFaceCentre.y) / effectiveRadius * (counterClockwise ? -1 : 1);
                double y = -(deflectedEndPoint.x - m_topFaceCentre.x) / effectiveRadius * (counterClockwise ? -1 : 1);
                const double a = sqrt((1 - z * z) / (x * x + y * y));
                // Resultant heading unit vector is a combination of the unaffected Z-component of the original heading
                // and the tangent vector to the cross-section m_crossSection @ deflectedEndPoint.
                unitHeadingVector = Vec3d(a * x, a * y, z); // No need to normalise; is already a unit vector.
            }
        }

        // Note that in this situation, we use the "approximation" vector, not the actual (updated) unitHeadingVector. 
        Vec3d approximatedHeading(deflectedEndPoint.x - intersectionPoint.x, deflectedEndPoint.y - intersectionPoint.y, deflectedEndPoint.z - intersectionPoint.z);
        // Furthermore, our effective magnitude is the distance between the two points.
        // We don't use the magintudeAfterIntersection, because that length is likely along the circumference. When that's the case,
        // travelling magnitudeAfterIntersection along the approximatedHeading takes us beyond the surface.

        // MSA 11.01.28 Capping effectiveMagnitude at magnitudeAfterIntersection (rounding errors may result in effectiveMagnitude being >= magnitudeAfterIntersection)

        const double effectiveMagnitude = Utility::CSMin(approximatedHeading.Length(), magnitudeAfterIntersection);

        approximatedHeading.Normalize();

        // MSA Note to self: Test how the lack of this code affects things
        /*
        // If we re-deflected, we may need to update the unitHeadingVector again.
        // However, first we should check to see that it's significantly different
        // (e.g. the result of intersection with a different surface of the cylinder).
        // If not, we shouldn't update, as floating point errors rapidly accumulate.
        if(        abs(unitHeadingVector.x - approximatedHeading.x)>0.005
            ||    abs(unitHeadingVector.y - approximatedHeading.y)>0.005
            ||    abs(unitHeadingVector.z - approximatedHeading.z)>0.005    )
        {
            unitHeadingVector = approximatedHeading;
        }
        */

        if (effectiveMagnitude > NEAR_AS_INTERSECTING)
        {
            endPoint = deflectedEndPoint;
        }
        else
        {
            // Note that - if deflectedEndPoint is a negligible distance from intersectionPoint - 
            // we want to use intersectionPoint as the endPoint.
            // This is because if we use deflectedEndPoint, those floating point errors really add up.
            endPoint = intersectionPoint;
        }

        if (Contains(startPoint)) return DeflectionToInside;
        // else
        return DeflectionToOutside;
    }

    const double BoundingCylinder::GetRootPenetrationProbability(const Surface& surfaceToTest, const double& angleBetweenRootAndNormal) const
    {
        double maxRPP = 0.0;
        switch (surfaceToTest)
        {
        case Top:
            maxRPP = m_topRPP;
            break;
        case Bottom:
            maxRPP = m_bottomRPP;
            break;
        case Curved:
            maxRPP = m_sideRPP;
            break;
        case Left:
        case Right:
        case Front:
        case Back:
        case NONE:
        default:
            maxRPP = 0.0;
            break;
        }

        switch (m_deflectionAlgorithm)
        {
        case CosinProportional:
        {
            if (maxRPP == 1) return 1;
            double RPP = maxRPP * cos(PI - angleBetweenRootAndNormal);
            return fabs(RPP);
        }
        case DirectlySpecified:
        default:
            return maxRPP;
        }
    }
} /* namespace rootmap */
