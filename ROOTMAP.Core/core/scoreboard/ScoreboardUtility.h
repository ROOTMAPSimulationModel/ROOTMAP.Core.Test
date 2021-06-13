#ifndef Scoreboard_Utility_H
#define Scoreboard_Utility_H

#include "core/macos_compatibility/macos_compatibility.h"
//#include "core/scoreboard/ScoreboardStratum.h"
#include "core/scoreboard/Dimension.h"
#include "core/scoreboard/ScoreboardBox.h"

namespace rootmap
{
    namespace ScoreboardUtility
    {
        // DEPRECATED - throws a std::logic_error
        // Use either BoxCoordinate constructor or BoxCoordinate::set(...)
        //void SetCoordinate (BoxCoordinate * coordinate, long x, long y, long z);

        // DEPRECATED - throws a std::logic_error
        // Use BoxCoordinate::IsValid()
        //Boolean IsValidCoordinate (BoxCoordinate *coordinate);

        bool IsValidCell(Cell* cell);
        bool IsValidBox(const BoxCoordinate& coordinate1, const BoxCoordinate& coordinate2);

        // DEPRECATED - throws a std::logic_error
        // Use BoxCoordinate::operator==()
        //Boolean EqualCoordinate (BoxCoordinate* coordinate1, BoxCoordinate* coordinate2);

        void GetBoxSize(BoxCoordinate* startcoordinate, BoxCoordinate* endcoordinate, BoxCoordinate* boxSize);
        bool PlaneIsInVolume(long plane, ViewDirection aViewDir, const BoxCoordinate& coordinate1, const BoxCoordinate& coordinate2);

#ifdef CS_UTILITIES_USING_PROJECTIONOF_FUNCTIONS
        void ProjectionOfBoxCoordinateOntoPoint(BoxCoordinate *coordinate, Point *pnt, ViewDirection viewdir);
        void ProjectionOfPointOntoBoxCoordinate(Point *pnt, BoxCoordinate *coordinate, ViewDirection viewdir);
        void ProjectionOfPointOntoBoxCoordinate(short h, short v, BoxCoordinate *coordinate, ViewDirection viewdir);
        void ProjectionOfLongIndexOntoPt(BoxCoordinate *idx, LongPt *pnt, ViewDirection viewdir);
        void ProjectionOfLongPtOntoIndex(LongPt *pnt, BoxCoordinate *idx, ViewDirection viewdir);
        void ProjectionOfLongPtOntoIndex(long h, long v, BoxCoordinate *idx, ViewDirection viewdir);
#endif // #ifdef CS_UTILITIES_USING_PROJECTIONOF_FUNCTIONS

        ViewDirection Dimension2ViewDirection(Dimension theDimension);
        Dimension ViewDirection2Dimension(ViewDirection viewdir);
    } // namespace ScoreboardUtility
} /* namespace rootmap */

#endif // #ifndef Scoreboard_Utility_H
