#include "simulation/process/modules/BoundingRectangularPrism.h"
#include "simulation/process/common/ProcessDrawing.h"

namespace rootmap
{
    using namespace boost::geometry;
    typedef std::vector<P> PV;
    typedef model::box<Pt> Box2D;

    BoundingRectangularPrism::BoundingRectangularPrism(const size_t index, const DoubleCoordinate& leftFrontTop, const DoubleCoordinate& rightBackBottom, const std::string& algo, const double& topRPP, const double& bottomRPP, const double& sideRPP, const double& topP, const double& bottomP, const double& sideP)
        : m_drawing(__nullptr)
        , m_index(index)
        , DoubleBox(leftFrontTop.x
            , leftFrontTop.y
            , leftFrontTop.z
            , rightBackBottom.x
            , rightBackBottom.y
            , rightBackBottom.z)
        , m_width(abs(right - left))
        , m_height(abs(back - front))
        , m_depth(abs(bottom - top))
        , m_leftFrontTop(leftFrontTop)
        , m_rightBackBottom(rightBackBottom)
        , m_TDC(left + (right - left) / 2.0, front + (back - front) / 2.0, top)
        , m_BDC(left + (right - left) / 2.0, front + (back - front) / 2.0, bottom)
        , m_squareCrossSection(right - left == back - front)
        , m_cachedIntersectionPointValid(false)
        , m_doubleIntersection(false)
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
        append(m_crossSection, make<Pt>(right, front));
        append(m_crossSection, make<Pt>(right, back));
        append(m_crossSection, make<Pt>(left, back));
        append(m_crossSection, make<Pt>(left, front));
        append(m_crossSection, make<Pt>(right, front));
        correct(m_crossSection);
    }

    BoundingRectangularPrism::~BoundingRectangularPrism()
    {
    }

    void BoundingRectangularPrism::SetDrawing(ProcessDrawing* drawing)
    {
        m_drawing = drawing;
    }

    void BoundingRectangularPrism::DrawScoreboard(const DoubleRect& /* area */, Scoreboard* /* scoreboard */)
    {
        if (m_drawing != __nullptr)
        {
            if (m_squareCrossSection)
            {
                static wxColour c(255, 0, 0, 180);
                const double radius = 0.5 * sqrt(2 * (right - left) * (right - left));
                m_drawing->DrawCone(m_TDC, m_BDC, radius, radius, c, vNONE, 4);
            }
            else
            {
                m_drawing->DrawRectangle(*this, vTop, bottom);
                m_drawing->DrawRectangle(*this, vFront, back);
                m_drawing->DrawRectangle(*this, vSide, right);
                m_drawing->DrawRectangle(*this, vSide, left);
                m_drawing->DrawRectangle(*this, vFront, front);
                m_drawing->DrawRectangle(*this, vTop, top);
            }
        }
    }

    long int BoundingRectangularPrism::DoRaytracerOutput(RaytracerData* raytracerData)
    {
        if (!m_raytracerDataHasBeenOutput && raytracerData != __nullptr)
        {
            raytracerData->AddRectangularPrism(*this);
            m_raytracerDataHasBeenOutput = true;
        }

        return kNoError;
    }

    const bool BoundingRectangularPrism::Contains(const DoubleCoordinate& point) const
    {
        if (point.x > left && point.x < right)
        {
            if (point.y > front && point.y < back)
            {
                if (point.z > top && point.z < bottom)
                {
                    return true;
                }
            }
        }
        return false;
    }

    const bool BoundingRectangularPrism::Contains(const P& crossSectionalPolygon, const double& z) const
    {
        if (z > top && z < bottom)
        {
            return within(crossSectionalPolygon, m_crossSection);
        }
        return false;
    }


    const bool BoundingRectangularPrism::Within(const VolumeObject* vo) const
    {
        if (vo == __nullptr)
        {
            return false;
        }
        std::vector<DoubleCoordinate> extrema = {
            DoubleCoordinate(left, top, front),
            DoubleCoordinate(left, top, back),
            DoubleCoordinate(left, bottom, front),
            DoubleCoordinate(left, bottom, back),
            DoubleCoordinate(right, top, front),
            DoubleCoordinate(right, top, back),
            DoubleCoordinate(right, bottom, front),
            DoubleCoordinate(right, bottom, back)
        };
        bool containsAll = true;
        for (size_t i = 0; containsAll && i < extrema.size(); ++i)
        {
            bool containsThis = vo->Contains(extrema[i]);
            containsAll = containsAll && containsThis;
        }

        return containsAll;
    }

    // MSA 11.02.16 Trying a new "jitter-lock" approach for simplicity/efficiency
    const DoubleCoordinate BoundingRectangularPrism::GenerateRandomPointInBox(const DoubleBox& box, const bool& insideThisVO)
    {
        DoubleCoordinate dc;

        const double voLeft = m_leftFrontTop.x;
        const double voRight = m_rightBackBottom.x;
        const double voFront = m_leftFrontTop.y;
        const double voBack = m_rightBackBottom.y;
        const double voTop = m_leftFrontTop.z;
        const double voBottom = m_rightBackBottom.z;

        bool useJitterLock = false;

        if (insideThisVO)
        {
            const double xmin = Utility::CSMax(voLeft, box.left);
            const double xmax = Utility::CSMin(voRight, box.right);
            const double ymin = Utility::CSMax(voFront, box.front);
            const double ymax = Utility::CSMin(voBack, box.back);
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
                    if (cp <= 0.001)
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

            if (iterations > 1000)
            {
                // this cannot be!
                RmAssert(false, "How did this happen?");
            }

            if (useJitterLock)
            {
                while (!(dc.z > voTop && dc.z < voBottom))
                {
                    dc.z = Utility::RandomRange(box.bottom, box.top);
                }
                while (!(dc.y > voFront && dc.y < voBack))
                {
                    dc.y = Utility::RandomRange(box.back, box.front);
                }
                while (!(dc.x > voLeft && dc.x < voRight))
                {
                    dc.x = Utility::RandomRange(box.right, box.left);
                }
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

            RmAssert(zmin<voTop || zmax>voBottom || xmin<voLeft || xmax>voRight || ymin<voFront || ymax>voBack, "This box is 100% coincident with this BoundingRectangularPrism; cannot generate a coordinate within the box but not within the BoundingRectangularPrism");

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
                    if (cp > 0.999)
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

            // MSA this is buggy in edge conditions
            if (useJitterLock)
            {
                while ((dc.z > voTop && dc.z < voBottom))
                {
                    dc.z = Utility::RandomRange(zmax, zmin);
                }
                while ((dc.y > voFront && dc.y < voBack))
                {
                    dc.y = Utility::RandomRange(ymax, ymin);
                }
                while ((dc.x > voLeft && dc.x < voRight))
                {
                    dc.x = Utility::RandomRange(xmax, xmin);
                }
            }
        }

        return dc;
    }


    const double BoundingRectangularPrism::GetCoincidentProportion(const DoubleBox& box, TransferRestrictedDimensionBitset& transferRestrictedDimensions, TransferRestrictedDimensionPermeabilities& transferRestrictedDimensionPermeabilities)
    {
        if ((*(static_cast<const DoubleBox*>(this)) == box))
        {
            transferRestrictedDimensions.set(); // Set all bits to 1.
            if (m_topP > 0.0) transferRestrictedDimensionPermeabilities[ZNegative] = m_topP; // Transfer upwards out the top
            if (m_topP > 0.0) transferRestrictedDimensionPermeabilities[ZPositive] = m_bottomP; // Transfer downwards out the bottom
            if (m_sideP > 0.0)
            {
                transferRestrictedDimensionPermeabilities[XNegative] = m_sideP; // Transfer leftwards out the left
                transferRestrictedDimensionPermeabilities[XPositive] = m_sideP; // Transfer rightwards out the right
                transferRestrictedDimensionPermeabilities[YNegative] = m_sideP; // Transfer forwards out the front
                transferRestrictedDimensionPermeabilities[YPositive] = m_sideP; // Transfer backwards out the back
            }
            return 1.0;
        }

        const Pt outsideRightBack(box.right + NEAR_AS_INTERSECTING, box.back + NEAR_AS_INTERSECTING);
        const Pt outsideLeftBack(box.left - NEAR_AS_INTERSECTING, box.back + NEAR_AS_INTERSECTING);
        const Pt outsideLeftFront(box.left - NEAR_AS_INTERSECTING, box.front - NEAR_AS_INTERSECTING);
        const Pt outsideRightFront(box.right + NEAR_AS_INTERSECTING, box.front - NEAR_AS_INTERSECTING);
        Box2D outsideDbXYBox(outsideLeftFront, outsideRightBack);

        const Pt rightBack(box.right, box.back);
        const Pt leftBack(box.left, box.back);
        const Pt leftFront(box.left, box.front);
        const Pt rightFront(box.right, box.front);
        Box2D dbXYBox(leftFront, rightBack);

        // MSA TODO Are these gradients always 0 and [infinity] by definition? No. Boxes are arbitrarily defined.
        // MSA 11.07.06 Slopes are the same for the normal lines and the "outside" lines (they are parallel).
        const double slopeL = (leftBack.get<1>() - leftFront.get<1>()) / (leftBack.get<0>() - leftFront.get<0>());
        const double slopeR = (rightBack.get<1>() - rightFront.get<1>()) / (rightBack.get<0>() - rightFront.get<0>());
        const double slopeF = (rightFront.get<1>() - leftFront.get<1>()) / (rightFront.get<0>() - leftFront.get<0>());
        const double slopeB = (rightBack.get<1>() - leftBack.get<1>()) / (rightBack.get<0>() - leftBack.get<0>());

        //const double outsideDbXYBoxArea = area(outsideDbXYBox);
        const double dbXYBoxArea = area(dbXYBox);
        const double brpXYPArea = area(m_crossSection);

        P dbXYP;
        append(dbXYP, rightFront);
        append(dbXYP, rightBack);
        append(dbXYP, leftBack);
        append(dbXYP, leftFront);
        correct(dbXYP);

        PV outsideVecXY;
        boost::geometry::intersection(outsideDbXYBox, m_crossSection, outsideVecXY);

        PV vecXY;
        boost::geometry::intersection(dbXYBox, m_crossSection, vecXY);

        double clippedVolume = 0;
        //const double outsideBoxVolume = //
        const double boxVolume = box.GetVolume();
        bool addXPos = false, addXNeg = false, addYPos = false, addYNeg = false, addZPos = false, addZNeg = false;
        bool leftIntersected = false, leftMatched = false, rightIntersected = false, rightMatched = false;
        bool frontIntersected = false, frontMatched = false, backIntersected = false, backMatched = false;
        PV::iterator oit = outsideVecXY.begin();
        for (PV::iterator it = vecXY.begin(); it != vecXY.end() && oit != outsideVecXY.end(); ++it, ++oit)
        {
            const double thisXYArea = area(*it);

            if (thisXYArea > 0)
            {
                bool completeXYOverlap;

                // N.B. For rectangles, if            C = Intersection(A, B);
                //        C is rectangular. Then if
                //                                    area(C) == area(A)
                //        B completely overlaps A,
                //        or if
                //                                    area(C) == area(B)
                //        A completely overlaps B.

                if (dbXYBoxArea < brpXYPArea)
                {
                    completeXYOverlap = equals(*it, dbXYP) || thisXYArea == dbXYBoxArea; // Small ScoreboardBox in big BoundingRectangularPrism
                }
                else
                {
                    completeXYOverlap = equals(*it, m_crossSection) || thisXYArea == brpXYPArea; // Small BoundingRectangularPrism in big ScoreboardBox
                }


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


                // Now, before checking the Z-axis, we need to check whether the "obstructions" as calculated
                // are due to the surfaces of the VO, or simple spatial coincidence.
                // This is important because spatial coincidence will not necessarily cause obstruction.
                // For example, if box A is 100% spatially coincident with a VO and box B next to it is 0%,
                // transfer from A to B & vice versa will be obstructed by the surface of the VO.
                // However, if box A is 100% spatially coincident with a VO and box B next to it is >0%,
                // transfer from A to [part of] B is unrestricted and the restriction should take place within B.

                bool leftCoplanarWithSurface = leftMatched, rightCoplanarWithSurface = rightMatched, frontCoplanarWithSurface = frontMatched, backCoplanarWithSurface = backMatched;

                if (leftCoplanarWithSurface || rightCoplanarWithSurface || frontCoplanarWithSurface || backCoplanarWithSurface)
                {
                    const double oL = outsideLeftFront.get<0>();
                    const double oR = outsideRightBack.get<0>();
                    const double oF = outsideLeftFront.get<1>();
                    const double oB = outsideRightBack.get<1>();

                    model::linestring<Pt> outsidePolygonLines(oit->outer().begin(), oit->outer().end());

                    model::linestring<Pt>::const_iterator outsidePolygonIter = outsidePolygonLines.begin();
                    Pt point1 = *outsidePolygonIter;
                    ++outsidePolygonIter; // This code assumes at least 2 valid points in the linestring. There logically must be at least 3 points, as thisArea>0.
                    do
                    {
                        Pt point2 = *(outsidePolygonIter);

                        const double p1x = point1.get<0>();
                        const double p1y = point1.get<1>();
                        const double p2x = point2.get<0>();
                        const double p2y = point2.get<1>();

                        if (leftCoplanarWithSurface && (p1x == oL) && (p2x == oL))
                        {
                            // This BoundingRectangularPrism extends beyond the parameter DoubleBox in the negative-X direction
                            leftCoplanarWithSurface = false;
                        }
                        if (rightCoplanarWithSurface && (p1x == oR) && (p2x == oR))
                        {
                            // This BoundingRectangularPrism extends beyond the parameter DoubleBox in the positive-X direction
                            rightCoplanarWithSurface = false;
                        }
                        if (frontCoplanarWithSurface && (p1y == oF) && (p2y == oF))
                        {
                            // This BoundingRectangularPrism extends beyond the parameter DoubleBox in the negative-Y direction
                            frontCoplanarWithSurface = false;
                        }
                        if (backCoplanarWithSurface && (p1y == oB) && (p2y == oB))
                        {
                            // This BoundingRectangularPrism extends beyond the parameter DoubleBox in the positive-Y direction
                            backCoplanarWithSurface = false;
                        }

                        point1 = point2;
                    } while (++outsidePolygonIter != outsidePolygonLines.end());
                }

                double coincidentZProportion = 0.0;

                // This conditional evaluates to true if the box intersects the top of the BoundingRectangularPrism OR the boxtop or boxbottom is coplanar with same
                if (box.top <= top)
                {
                    if (box.bottom == top)
                    {
                        // When the box is "sitting on top of" the VO, all of the box is transferable upwards.
                        addZNeg = false;
                        // If and only if there is less than complete overlap, all of the box is transferable downwards 
                        // (because everything in the box can, theoretically, get through the noncoincident section of the coplanar surfaces).
                        if (completeXYOverlap) addZPos = true;

                        coincidentZProportion = 0.0;
                        frontCoplanarWithSurface = false;
                        backCoplanarWithSurface = false;
                        leftCoplanarWithSurface = false;
                        rightCoplanarWithSurface = false;

                        frontIntersected = false;
                        backIntersected = false;
                        leftIntersected = false;
                        rightIntersected = false;

                        frontMatched = false;
                        backMatched = false;
                        leftMatched = false;
                        rightMatched = false;
                    }
                    else if (box.bottom > top)
                    {
                        if (completeXYOverlap)
                        {
                            // If there is complete overlap between the XY cross-section of the BoundingRectangularPrism and this box, then both Z-directions are obstructed.
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
                        coincidentZProportion = (box.bottom - top) / (box.bottom - box.top);
                    }
                }
                // This conditional evaluates to true if the box intersects the bottom of the BoundingRectangularPrism OR the boxtop or boxbottom is coplanar with same
                if (box.bottom >= bottom)
                {
                    if (box.top == bottom)
                    {
                        // If there is complete overlap between the XY cross-section of the BoundingRectangularPrism and this box, then the Z-direction is obstructed.
                        // When the VO is "sitting on top of" the box, all of the box is transferable downwards.
                        addZPos = false;
                        // If and only if there is less than complete overlap, all of the box is transferable upwards 
                        // (because everything in the box can, theoretically, get through the noncoincident section of the coplanar surfaces).
                        if (completeXYOverlap) addZNeg = true;

                        coincidentZProportion = 0.0;
                        frontCoplanarWithSurface = false;
                        backCoplanarWithSurface = false;
                        leftCoplanarWithSurface = false;
                        rightCoplanarWithSurface = false;

                        frontIntersected = false;
                        backIntersected = false;
                        leftIntersected = false;
                        rightIntersected = false;

                        frontMatched = false;
                        backMatched = false;
                        leftMatched = false;
                        rightMatched = false;
                    }
                    else if (box.top <= bottom)
                    {
                        if (completeXYOverlap)
                        {
                            // If there is complete overlap between the XY cross-section of the BoundingRectangularPrism and this box, then both Z-directions are obstructed.
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
                        coincidentZProportion = (bottom - box.top) / (box.bottom - box.top);
                    }
                }
                // Is the box entirely within the BRP?
                if (box.top >= top && box.bottom <= bottom)
                {
                    coincidentZProportion = 1.0;
                }

                // Now, we evaluate the transfer restriction applicable to each face of this Box due to interaction with this BoundingRectangularPrism.

                // Asterisk refers to this note:
                // * (and whatever will permeate through the VO surface)

                // X-wise:
                if (leftCoplanarWithSurface) addXNeg = true;
                if (rightCoplanarWithSurface) addXPos = true;
                if (leftIntersected && !rightIntersected) addXPos = true; // VO intersects with the left side only; transfer out rightwards is restricted to the noncoincident section*
                if (rightIntersected && !leftIntersected) addXNeg = true; // VO intersects with the right side only; transfer out leftwards is restricted to the noncoincident section*
                if (coincidentZProportion == 1.0)
                {
                    if (leftMatched && ((rightIntersected && !rightMatched) || (!rightIntersected))) addXNeg = true; // VO blocks whole left side but only part or none of right side; transfer out leftwards is restricted to the coincident section*
                    if (rightMatched && ((leftIntersected && !leftMatched) || (!leftIntersected))) addXPos = true; // VO blocks whole right side but only part or none of left side; transfer out rightwards is restricted to the coincident section*
                }
                // Y-wise:
                if (frontCoplanarWithSurface) addYNeg = true;
                if (backCoplanarWithSurface) addYPos = true;
                if (frontIntersected && !backIntersected) addYPos = true; // VO intersects with the front side only; transfer out backwards is restricted to the noncoincident section*
                if (backIntersected && !frontIntersected) addYNeg = true; // VO intersects with the back side only; transfer out frontwards is restricted to the noncoincident section*
                if (coincidentZProportion == 1.0)
                {
                    if (frontMatched && ((backIntersected && !backMatched) || (!backIntersected))) addYNeg = true; // VO blocks whole front side but only part or none of back side; transfer out frontwards is restricted to the coincident section*
                    if (backMatched && ((frontIntersected && !frontMatched) || (!frontIntersected))) addYPos = true; // VO blocks whole back side but only part or none of front side; transfer out backwards is restricted to the coincident section*
                }

                clippedVolume += thisXYArea * coincidentZProportion * (box.bottom - box.top);
            } // end if(thisXYArea>0)
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

        // To avoid throwing an assertion error just because of floating point inaccuracy
        if (clippedVolume > boxVolume && clippedVolume - boxVolume < 1e-12)
        {
            clippedVolume = boxVolume;
        }
        RmAssert(clippedVolume <= boxVolume, "Logic error: TARDIS-type inner/outer spatial mismatch");

        return clippedVolume / boxVolume;
    }

    // Returns the closest intersection, if any, between the line segment origin-->destination and this BoundingRectangularPrism.
    const bool BoundingRectangularPrism::GetClosestIntersection(const DoubleCoordinate& origin, const DoubleCoordinate& destination, DoubleCoordinate& intersectionDC, Vec3d& normalToIntersectedSurface, Surface& intersectedSurface)
    {
        if (origin.x < left && destination.x < left)
        {
            return false;
        }
        if (origin.x > right && destination.x > right)
        {
            return false;
        }
        if (origin.y < front && destination.y < front)
        {
            return false;
        }
        if (origin.y > back && destination.y > back)
        {
            return false;
        }
        if (origin.z < top && destination.z < top)
        {
            return false;
        }
        if (origin.z > bottom && destination.z > bottom)
        {
            return false;
        }

        Surface workingSurface = Top;

        // Obtain intersection of line and plane using parametric matrix algebra method
        // See: http://en.wikipedia.org/wiki/Line-plane_intersection

        // This is the parameter for the parametric form of the (origin --> destination) line segment. 
        // Whichever plane is intersected with the smallest (>=0) t is the first intersected.
        double t = 1e9;
        bool inside = true;
        Vec3d tempNormal(0, 0, 0);

        // 1) Check for intersection with XY plane.
        //-----------------------------------------------------------------------------------------------
        if (origin.z < top && destination.z > origin.z && destination.z >= top)
        {
            // Intersection with top plane, from outside
            tempNormal.z = -1;
            inside = false;
            workingSurface = Top;
        }
        else if (origin.z > top && destination.z < origin.z && destination.z <= top)
        {
            // Intersection with top plane, from inside
            tempNormal.z = 1;
            inside = true;
            workingSurface = Top;
        }
        else if (origin.z < bottom && destination.z > origin.z && destination.z >= bottom)
        {
            // Intersection with bottom plane, from inside
            tempNormal.z = -1;
            inside = true;
            workingSurface = Bottom;
        }
        else if (origin.z > bottom && destination.z < origin.z && destination.z <= bottom)
        {
            // Intersection with bottom plane, from outside
            tempNormal.z = 1;
            inside = false;
            workingSurface = Bottom;
        }

        // If intersection found, calculate its coordinates.
        if (tempNormal.z != 0)
        {
            // Choose simple arbitrary (but non-colinear) points on the plane: (0,0,top), (1,0,top) and (0,1,top) [or] (0,0,bottom), (1,0,bottom) and (0,1,bottom).
            // This matrix is valid for all parallel planes where Z=k, so we can use it for the top and bottom planes.

            const double det = origin.z - destination.z; // from simplifying the algebraic form, determinant shakes out to be za-zb.

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

            double vectorToPlane[3] = { origin.x, // line xa - plane x0
                origin.y, // line ya - plane y0
                -1e100 }; // "uninitialised" flag value

            if (inside && tempNormal.z > 0) // Intersecting top plane from below
            {
                vectorToPlane[2] = origin.z - top;
            }
            else if (inside) // Intersecting bottom plane from above
            {
                vectorToPlane[2] = origin.z - bottom;
            }
            else if (tempNormal.z > 0) // Intersecting bottom plane from below
            {
                vectorToPlane[2] = origin.z - bottom;
            }
            else // Intersecting top plane from above
            {
                vectorToPlane[2] = origin.z - top;
            }

            const double thisT = inv[0][0] * vectorToPlane[0] + inv[1][0] * vectorToPlane[1] + inv[2][0] * vectorToPlane[2];

            if (thisT >= 0 && thisT < 1)
            {
                // Find X & Y coordinates of plane intersection point
                const double x = origin.x + thisT * (destination.x - origin.x);
                const double y = origin.y + thisT * (destination.y - origin.y);
                if (x >= left && x <= right && y >= front && y <= back)
                {
                    t = thisT;
                    normalToIntersectedSurface = tempNormal;
                    intersectionDC.x = x;
                    intersectionDC.y = y;
                    intersectionDC.z = origin.z + t * (destination.z - origin.z);
                    intersectedSurface = workingSurface;
                }
            }
            tempNormal = Vec3d(0, 0, 0);
        }

        // 2) Check for intersection with XZ plane.
        //-----------------------------------------------------------------------------------------------
        if (origin.y < front && destination.y > origin.y && destination.y >= front)
        {
            // Intersection with front plane, from outside
            tempNormal.y = -1;
            inside = false;
            workingSurface = Front;
        }
        else if (origin.y > front && destination.y < origin.y && destination.y <= front)
        {
            // Intersection with front plane, from inside
            tempNormal.y = 1;
            inside = true;
            workingSurface = Front;
        }
        else if (origin.y < back && destination.y > origin.y && destination.y >= back)
        {
            // Intersection with back plane, from inside
            tempNormal.y = -1;
            inside = true;
            workingSurface = Back;
        }
        else if (origin.y > back && destination.y < origin.y && destination.y <= back)
        {
            // Intersection with back plane, from outside
            tempNormal.y = 1;
            inside = false;
            workingSurface = Back;
        }

        // If intersection found, calculate its coordinates.
        if (tempNormal.y != 0)
        {
            // Choose simple arbitrary (but non-colinear) points on the plane: (0,front,0), (1,front,0) and (0,front,1) [or] (0,back,0), (1,back,0) and (0,back,1).
            // This matrix is valid for all parallel planes where Y=k, so we can use it for the front and back planes.

            const double det = destination.y - origin.y; // from simplifying the algebraic form, determinant shakes out to be yb-ya.

            // No need to define the original matrix - inverse has been calculated manually and tested.
            double inv[3][3];
            inv[0][0] = 0;
            inv[0][1] = 1;
            inv[0][2] = 0;
            inv[1][0] = -1.0 / det;
            inv[1][1] = (origin.x - destination.x) / det;
            inv[1][2] = (origin.z - destination.z) / det;
            inv[2][0] = 0;
            inv[2][1] = 0;
            inv[2][2] = 1;

            double vectorToPlane[3] = { origin.x, // line xa - plane x0
                -1e100, // "uninitialised" flag value
                origin.z }; // line za - plane z0        

            if (inside && tempNormal.y > 0) // Intersecting front plane from behind
            {
                vectorToPlane[1] = origin.y - front;
            }
            else if (inside) // Intersecting back plane from in front
            {
                vectorToPlane[1] = origin.y - back;
            }
            else if (tempNormal.y > 0) // Intersecting back plane from behind
            {
                vectorToPlane[1] = origin.y - back;
            }
            else // Intersecting front plane from in front
            {
                vectorToPlane[1] = origin.y - front;
            }

            double thisT = inv[0][0] * vectorToPlane[0] + inv[1][0] * vectorToPlane[1] + inv[2][0] * vectorToPlane[2];

            if (thisT >= 0 && thisT < 1)
            {
                // Find X & Z coordinates of plane intersection point
                const double x = origin.x + thisT * (destination.x - origin.x);
                const double z = origin.z + thisT * (destination.z - origin.z);
                if (x >= left && x <= right && z >= top && z <= bottom && thisT < t)
                {
                    t = thisT;
                    normalToIntersectedSurface = tempNormal;
                    intersectionDC.x = x;
                    intersectionDC.y = origin.y + t * (destination.y - origin.y);
                    intersectionDC.z = z;
                    intersectedSurface = workingSurface;
                }
            }
            tempNormal = Vec3d(0, 0, 0);
        }

        // 3) Check for intersection with YZ plane.
        //-----------------------------------------------------------------------------------------------
        if (origin.x < left && destination.x > origin.x && destination.x >= left)
        {
            // Intersection with left plane, from outside
            tempNormal.x = -1;
            inside = false;
            workingSurface = Left;
        }
        else if (origin.x > left && destination.x < origin.x && destination.x <= left)
        {
            // Intersection with left plane, from inside
            tempNormal.x = 1;
            inside = true;
            workingSurface = Left;
        }
        else if (origin.x < right && destination.x > origin.x && destination.x >= right)
        {
            // Intersection with right plane, from inside
            tempNormal.x = -1;
            inside = true;
            workingSurface = Right;
        }
        else if (origin.x > right && destination.x < origin.x && destination.x <= right)
        {
            // Intersection with right plane, from outside
            tempNormal.x = 1;
            inside = false;
            workingSurface = Right;
        }

        // If intersection found, calculate its coordinates.
        if (tempNormal.x != 0)
        {
            // Choose simple arbitrary (but non-colinear) points on the plane: (left,0,0), (left,1,0) and (left,0,1) [or] (right,0,0), (right,1,0) and (right,0,1).
            // This matrix is valid for all parallel planes where X=k, so we can use it for the left and right planes.

            const double det = origin.x - destination.x; // from simplifying the algebraic form, determinant shakes out to be xa-xb.

            // No need to define the original matrix - inverse has been calculated manually and tested.
            double inv[3][3];
            inv[0][0] = 1.0 / det;
            inv[0][1] = (destination.y - origin.y) / det;
            inv[0][2] = (destination.z - origin.z) / det;
            inv[1][0] = 0;
            inv[1][1] = 1;
            inv[1][2] = 0;
            inv[2][0] = 0;
            inv[2][1] = 0;
            inv[2][2] = 1;

            double vectorToPlane[3] = { -1e100, // "uninitialised" flag value
                origin.y, // line ya - plane y0
                origin.z }; // line za - plane z0

            if (inside && tempNormal.x > 0) // Intersecting left plane from the right
            {
                vectorToPlane[0] = origin.x - left;
            }
            else if (inside) // Intersecting right plane from the left
            {
                vectorToPlane[0] = origin.x - right;
            }
            else if (tempNormal.x > 0) // Intersecting right plane from the right
            {
                vectorToPlane[0] = origin.x - right;
            }
            else // Intersecting left plane from the left
            {
                vectorToPlane[0] = origin.x - left;
            }

            double thisT = inv[0][0] * vectorToPlane[0] + inv[1][0] * vectorToPlane[1] + inv[2][0] * vectorToPlane[2];

            if (thisT >= 0 && thisT < 1)
            {
                // Find Y & Z coordinates of plane intersection point
                const double y = origin.y + thisT * (destination.y - origin.y);
                const double z = origin.z + thisT * (destination.z - origin.z);
                if (y >= front && y <= back && z >= top && z <= bottom && thisT < t)
                {
                    t = thisT;
                    normalToIntersectedSurface = tempNormal;
                    intersectionDC.x = origin.x + t * (destination.x - origin.x);
                    intersectionDC.y = y;
                    intersectionDC.z = z;
                    intersectedSurface = workingSurface;
                }
            }
        }

        if (t >= 0 && t < 1)
        {
            return true;
        }

        return false;
    }

    VolumeObject::InteractionType BoundingRectangularPrism::DeflectRootSegment(const DoubleCoordinate& startPoint, DoubleCoordinate& intersectionPoint, DoubleCoordinate& endPoint, Vec3d& unitHeadingVector, const double& rootRadius, const long& /*branch_order*/)
    {
        Vec3d normalToIntersectedSurface;
        Surface intersectedSurface = NONE;

        if (!GetClosestIntersection(startPoint, endPoint, intersectionPoint, normalToIntersectedSurface, intersectedSurface))
        {
            // This root segment did not encounter any surfaces.
            return NoInteraction;
        }

        // So an intersection has been found. Can the root penetrate the surface at this intersection?
        double p = GetRootPenetrationProbability(intersectedSurface, unitHeadingVector.angleBetween(normalToIntersectedSurface));
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


        normalToIntersectedSurface.Normalize();
        if (normalToIntersectedSurface.dotProd(unitHeadingVector) < 0)
        {
            // Root segment vector is inbound to surface.
            // First: check if it's inbound at exactly along the normal
            const bool parallel = normalToIntersectedSurface.parallelTo(unitHeadingVector);
            // If so, we'll have to add a random component to the non-intersected directions to avoid getting a zero vector.
            // Either way, we zero out the parallel component.

            if (normalToIntersectedSurface.x != 0)
            {
                unitHeadingVector.x = 0;
                if (parallel)
                {
                    unitHeadingVector.y = 0.5 - Utility::Random1();
                    // MSA 10.09.22 Temporary kludge: restricting randomised headings of roots to horizontal-only.
                    //if(branch_order!=0)    newHeading.z = 0.5 - Utility::Random1();
                }
            }
            else if (normalToIntersectedSurface.y != 0)
            {
                unitHeadingVector.y = 0;
                if (parallel)
                {
                    unitHeadingVector.x = 0.5 - Utility::Random1();
                    // MSA 10.09.22 Temporary kludge: restricting randomised headings of roots to horizontal-only.
                    //if(branch_order!=0)    newHeading.z = 0.5 - Utility::Random1();
                }
            }
            else if (normalToIntersectedSurface.z != 0)
            {
                unitHeadingVector.z = 0;
                if (parallel)
                {
                    unitHeadingVector.x = 0.5 - Utility::Random1();
                    unitHeadingVector.y = 0.5 - Utility::Random1();
                }
            }
            else
            {
                RmAssert(false, "Bad normal vector returned");
            }
        }
        // Else root segment vector is parallel to (dotProd==0) or outbound from (dotProd>0) the surface. newHeading==undeflectedUnitHeading.

        const double magnitudeAfterIntersection = Distance3D(&intersectionPoint, &endPoint);

        if (magnitudeAfterIntersection > NEAR_AS_INTERSECTING)
        {
            endPoint = intersectionPoint + magnitudeAfterIntersection * unitHeadingVector;
        }
        else
        {
            // Note that - if deflectedEndPoint is a negligible distance from intersectionPoint - 
            // we want to use intersectionPoint as the endPoint.
            // This is because if we use deflectedEndPoint, those floating point errors really add up.
            endPoint = intersectionPoint;
        }

        if (Contains(startPoint)) return DeflectionToInside;
        else
            return DeflectionToOutside;
    }

    const double BoundingRectangularPrism::GetRootPenetrationProbability(const Surface& surfaceToTest, const double& angleBetweenRootAndNormal) const
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
        case Left:
        case Right:
        case Front:
        case Back:
            maxRPP = m_sideRPP;
            break;
        case Curved:
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
