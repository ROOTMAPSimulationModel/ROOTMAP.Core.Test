#ifndef ProcessDrawing_H
#define ProcessDrawing_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessDrawing.h
// Purpose:     Declaration of the ProcessDrawing class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-19 19:34:59 +0800 (Sun, 19 Jul 2009) $
// $Revision: 72 $
// Copyright:   ©2006 University of Tasmania
//
// ProcessDrawing is an as-yet not properly named class to simplify process
// drawing and make it more robust.
//
// A Process "has a" ProcessDrawing IFF it needs to draw.
// 
// A IProcessDrawingBuffer will only contain a process if that process has a 
// ProcessDrawing.
//
// This class manages the IProcessDrawingBuffer instances that the process
// draws to. They are stored as a std::list because random removal may occur.
//
// There is no attempt made to assert that a process has a ProcessDrawing. If
// a process doesn't have a ProcessDrawing, it won't be contained in a buffer
// and won't be asked to Draw().
//
// If a Process doesn't have a ProcessDrawing but nevertheless attempts to call
// one of the buffer drawing functions, that is deemed a programmer error.
// Currently this is not asserted, however all drawing functions should be
// surrounded by the BEGIN_DRAWING and END_DRAWING macros to cater for this.
//
// Note that even if a process has a ProcessDrawing, that drawing may not be
// taking place - this may be if either
// a) there are no view windows open (including non-gui operation)
// b) there are no view windows containing the Process
//
//
//                          IMPORTANT NOTE:
//
// The definition/implementation of this class is at the app layer, for reasons
// explained there. For example, see
// app/gui/ViewWindow/ProcessDrawing.cpp
//
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/Types.h"
#include "core/common/Structures.h"
#include "core/common/DoubleCoordinates.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/macos_compatibility/macos_compatibility.h"

#include <list>

//class wxColour;
#ifndef WX_PRECOMP
#include "wx/colour.h"
#include "wx/gdicmn.h"
#endif // #ifndef WX_PRECOMP

namespace rootmap
{
    class IProcessDrawingBuffer;
    class ScoreboardCoordinator;
    class ScoreboardStratum;
    class Scoreboard;
    class Process;

    class ProcessDrawing
    {
    public:
        ProcessDrawing(Process* p);
        virtual ~ProcessDrawing();

        /**
         * May be useful for optimising drawing, particularly regarding the DC's
         *
         * @param
         * @return
         */
        void BeginDrawing(IProcessDrawingBuffer* buffer);

        void EndDrawing(IProcessDrawingBuffer* buffer);

        /**
         * Accessor for the owning process.
         *
         * @param
         * @return
         */
        Process* GetProcess();

        /**
         * Request for a redraw of one scoreboard to one buffer.
         *
         * @param
         * @return
         */
        void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard, IProcessDrawingBuffer* buffer);

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */

        virtual void AddDrawingBuffer(IProcessDrawingBuffer* buffer);
        virtual void RemoveDrawingBuffer(IProcessDrawingBuffer* buffer);

        /* Drawing commands.
        The reasons for their being defined again here, instead of overridding VBuffer, are:
        a) VBuffer's drawing routines are not virtual, to increase speed.
        b) Redefining them gives a chance to rename them more appropriately for public use
            thus and introduce a sensible set of routines.
        c) With overriding, some of the routines might have become VERY messy


        Without further ado, let us commence with the basic acts of moving to a
        point and drawing a line to a point */
        void MovePenTo(const DoubleCoordinate& fi);
        void DrawLineTo(const DoubleCoordinate& fi);
        void DrawLine(const DoubleCoordinate& from_coord, const DoubleCoordinate& to_coord); // "fi" specifies _delta_ values


        /* This function draws a rectangle in either the x, y, or z plane (YZ, XZ or XY).
        The "direction" parameter decides in which of the planes the rectangle will be drawn:
            vFront  =   XZ
            vSide   =   YZ
            vTop    =   XY (ie. horizontal)
        The "place" parameter decides how far along in the "direction" the rectangle goes. */
        void DrawRectangle(const ScoreboardStratum& stratum, BoxCoordinate* bc, ViewDirection direction, double place);

        /* This function also draws a rectangle in either the x, y, or z plane (YZ, XZ or XY).
        Here also, the "direction" parameter decides in which of the planes the rectangle will be drawn.
        The bounds of the rectangle are defined by the projection within the box defined by the two
        points given. If the rectangle is seen obliquely, it is seen at "place". Other documentation
        may be more explanatory.  */
        void DrawRectangle(const DoubleBox& soil_box, ViewDirection direction, double place);

        /* This function is similary to the last, however the place is found implicitly using the
        appropriate field of the DoubleBox's StartCoordinate */
        void DrawRectangle(const DoubleBox& soil_box, ViewDirection direction);

        /* This function is very similar to the last, however the view direction is found implicitly.
        If the two points given lie on one of the three prevailing orthogonal planes, it will be drawn
        as such. If the two coordinates are not in one of the three orthogonal planes, ie. they form a
        rectangulary prism (or even a cube), it is assumed that the rectangle is horizontal. That is,
        the 'z' value of the second point will be ignored. */
        void DrawRectangle(const DoubleBox& soil_box);

        /* Four oval drawing routines, similar in usage to the rectangle routines */
        void DrawOval(const ScoreboardStratum& stratum, BoxCoordinate* bc, ViewDirection direction, double place);
        void DrawOval(const DoubleBox& soil_box, ViewDirection direction, double place);
        void DrawOval(const DoubleBox& soil_box, ViewDirection direction);
        void DrawOval(const DoubleBox& soil_box);


        /* Drawing the dot, in the prevailing and selected colours */
        void DrawDot(DoubleCoordinate& fi);


        // Rectangles, Ovals and Dot, as above, but of a non-prevailing, ie. specified, colour !
        void DrawLineTo(const DoubleCoordinate& fi, wxColour& colour);
        void DrawLine(const DoubleCoordinate& coord1, const DoubleCoordinate& coord2, wxColour& colour); // "fi" specifies _delta_ values

        void DrawRectangle(const ScoreboardStratum& stratum, BoxCoordinate* bc, ViewDirection direction, double place, wxColour& colour);
        void DrawRectangle(const DoubleBox& soil_box, ViewDirection direction, double place, wxColour& colour);
        void DrawRectangle(const DoubleBox& soil_box, ViewDirection direction, wxColour& colour);
        void DrawRectangle(const DoubleBox& soil_box, wxColour& colour);

        void DrawOval(const ScoreboardStratum& stratum, BoxCoordinate* bc, ViewDirection direction, double place, wxColour& colour);
        void DrawOval(const DoubleBox& soil_box, ViewDirection direction, double place, wxColour& colour);
        void DrawOval(const DoubleBox& soil_box, ViewDirection direction, wxColour& colour);
        void DrawOval(const DoubleBox& soil_box, wxColour& colour);

        void DrawDot(DoubleCoordinate& fi, wxColour& colour);

        void RemoveDot(const DoubleCoordinate& fi);
        // MSA 11.04.14 New, much more efficient way of removing dots
        void RemoveAllDots();
        void RemoveRectangle(const ScoreboardStratum& stratum, BoxCoordinate* bc, ViewDirection direction, double place);

        void DrawCone(const DoubleCoordinate& coord1, const DoubleCoordinate& coord2, const double& radius1, const double& radius2, wxColour& colour, const ViewDirection& direction, const size_t& stacksNSlices = 64, const bool& wireframe = false);

        //NOT IMPLEMENTED
        //void DrawCylinder(const DoubleCoordinate & coord1, const DoubleCoordinate & coord2, const long & order, const PlantElementIdentifier & elid);
        //void DrawCylinder(const DoubleCoordinate & coord1, const DoubleCoordinate & coord2, const long & order, const PlantElementIdentifier & elid, const wxColour & colour);

        // Methods for receiving generalised-root-segment drawing instructions from Plant.cpp,
        // implementation will vary between buffer types.
        // A "root segment" is defined as a straight-line section of root, all the same order and all the same radius.
        // It may be a whole Branch, a whole RootTip, part of a Branch or part of a RootTip.
        //void DrawRootTo(const DoubleCoordinate & coord, const long & order, const PlantElementIdentifier & elid);
        //void DrawRootTo(const DoubleCoordinate & coord, const long & order, const PlantElementIdentifier & elid, const wxColour & colour);
        void DrawRootSegment(const DoubleCoordinate& coord1, const DoubleCoordinate& coord2, const long& order, const double& radius, const PlantElementIdentifier& elid);
        void DrawRootSegment(const DoubleCoordinate& coord1, const DoubleCoordinate& coord2, const double& radius, const PlantElementIdentifier& elid, wxColour& colour);


    private:
        ///
        /// type definition for a list of Process Buffers
        typedef std::list<IProcessDrawingBuffer *> ProcessBufferContainer;

        ///
        /// the drawing buffers
        ProcessBufferContainer m_drawingBuffers;

        ///
        /// the parent process
        Process* m_process;

        ///
        /// if one specific buffer has requested an update, only draw to that one
        IProcessDrawingBuffer* m_singleBuffer;
    }; // class ProcessDrawing

    inline Process* ProcessDrawing::GetProcess()
    {
        return m_process;
    }
} /* namespace rootmap */

#endif // #ifndef ProcessDrawing_H
