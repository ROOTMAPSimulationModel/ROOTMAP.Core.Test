#include "core/scoreboard/ScoreboardUtility.h"
#include "core/utility/Utility.h"
#include <stdexcept>

namespace rootmap
{
    namespace ScoreboardUtility
    {
        /* IsValidCell
        For a cell to be valid, all dimensions must have a value greater than or equal zero */
        bool IsValidCell(Cell* aCell)
        {
            return ((aCell->v >= 0) && (aCell->h >= 0));
        }


        /* IsValidBox
        For a box to be valid, the start and end vertices must be valid */
        bool IsValidBox(const BoxCoordinate& coordinate1, const BoxCoordinate& coordinate2)
        {
            return ((coordinate1.isValid()) && (coordinate2.isValid()));
        }


        /* GetBoxSize */
        void GetBoxSize(BoxCoordinate* startcoordinate, BoxCoordinate* endcoordinate, BoxCoordinate* boxSize)
        {
            BoxIndex t;
            //t = startcoordinate->x - endcoordinate->x;
            boxSize->x = Utility::CSAbs(startcoordinate->x - endcoordinate->x);
            //t = startcoordinate->y - endcoordinate->y;
            boxSize->y = Utility::CSAbs(startcoordinate->y - endcoordinate->y);
            t = startcoordinate->z - endcoordinate->z;
            boxSize->z = Utility::CSAbs(startcoordinate->z - endcoordinate->z);
        }


        /* LayerIsInBox */
        bool PlaneIsInVolume
        (long plane,
            ViewDirection aViewDir,
            const BoxCoordinate& coordinate1,
            const BoxCoordinate& coordinate2)
        {
            bool IsIn = false; // MSA 09.10.02 Initialised to false to silence compiler warning

            switch (aViewDir)
            {
            case vFront:
                if (coordinate1.y <= coordinate2.y)
                    IsIn = ((plane >= coordinate1.y) && (plane <= coordinate2.y));
                else
                    IsIn = ((plane >= coordinate2.y) && (plane <= coordinate1.y));
                break;

            case vSide:
                if (coordinate1.x <= coordinate2.x)
                    IsIn = ((plane >= coordinate1.x) && (plane <= coordinate2.x));
                else
                    IsIn = ((plane >= coordinate2.x) && (plane <= coordinate1.x));
                break;

            case vTop:
                if (coordinate1.z <= coordinate2.z)
                    IsIn = ((plane >= coordinate1.z) && (plane <= coordinate2.z));
                else
                    IsIn = ((plane >= coordinate2.z) && (plane <= coordinate1.z));
                break;
            case vNONE:
            default:
                break;
            }// switch (aViewDir)
            return (IsIn);
        }

#ifdef CS_UTILITIES_USING_PROJECTIONOF_FUNCTIONS
        //void ProjectionOfBoxCoordinateOntoPoint(BoxCoordinate *coordinate, Point *pnt, ViewDirection viewdir)
        //{
        //    switch (viewdir) {
        //        case vFront: pnt->h = coordinate->x; pnt->v = coordinate->z; break;
        //        case vSide: pnt->h = coordinate->y; pnt->v = coordinate->z; break;
        //        case vTop: pnt->h = coordinate->x; pnt->v = coordinate->y; break;
        //    }
        //}

        void ProjectionOfPointOntoBoxCoordinate(Point *pnt, BoxCoordinate *coordinate, ViewDirection viewdir)
        {
            ProjectionOfPointOntoBoxCoordinate(pnt->h, pnt->v, coordinate, viewdir);
        }

        void ProjectionOfPointOntoBoxCoordinate(short h, short v, BoxCoordinate *coordinate, ViewDirection viewdir)
        {
            switch (viewdir) {
            case vFront: coordinate->x = h; coordinate->z = v; break;
            case vSide: coordinate->y = h; coordinate->z = v; break;
            case vTop: coordinate->x = h; coordinate->y = v; break;
            }
        }

        void ProjectionOfLongIndexOntoPt(LongIndex *idx, LongPt *pnt, ViewDirection viewdir)
        {
            switch (viewdir) {
            case vFront: pnt->h = idx->x; pnt->v = idx->z; break;
            case vSide: pnt->h = idx->y; pnt->v = idx->z; break;
            case vTop: pnt->h = idx->x; pnt->v = idx->y; break;
            }
        }

        void ProjectionOfLongPtOntoIndex(LongPt *pnt, LongIndex *idx, ViewDirection viewdir)
        {
            ProjectionOfLongPtOntoIndex(pnt->h, pnt->v, idx, viewdir);
        }

        void ProjectionOfLongPtOntoIndex(long h, long v, LongIndex *idx, ViewDirection viewdir)
        {
            switch (viewdir) {
            case vFront: idx->x = h; idx->z = v; break;
            case vSide: idx->y = h; idx->z = v; break;
            case vTop: idx->x = h; idx->y = v; break;
            }
        }
#endif // #ifdef CS_UTILITIES_USING_PROJECTIONOF_FUNCTIONS

        /* _Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities_
        LayerIsInBox
        _Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities_ */
        ViewDirection Dimension2ViewDirection(Dimension theDimension)
        {
            return (ViewDirection)(theDimension);

            //    switch (theDimension) {
            //        case X: return vSide; break;
            //        case Y: return vFront; break;
            //        case Z: return vTop; break;
            //    }
            //    return vFront;
        }


        /* _Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities_
        LayerIsInBox
        _Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities__Utilities_ */
        Dimension ViewDirection2Dimension(ViewDirection theViewDir)
        {
            return static_cast<Dimension>(theViewDir);

            //    switch (theViewDir) {
            //        case vFront: return Y;
            //        case vSide: return X;
            //        case vTop: return Z;
            //    }
            //    return X;
            //
        }
    } // namespace ScoreboardUtility
} /* namespace rootmap */
