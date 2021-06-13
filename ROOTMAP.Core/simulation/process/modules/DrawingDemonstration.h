#ifndef DrawingDemonstration_H
#define DrawingDemonstration_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DrawingDemonstration.h
// Purpose:     Declaration of the DrawingDemonstration class
// Created:     10-07-2009
// Author:      RvH
// $Date$
// $Revision$
// Copyright:   ©2006-2009 University of Tasmania
//
/////////////////////////////////////////////////////////////////////////////
//other files whose definitions we use in this file
#include "simulation/process/common/Process.h"
#include "core/common/DoubleCoordinates.h"
// Rootmap log4cpp macros
#include "core/log/Logger.h"

// contains std::pair<>
#include <utility>

namespace rootmap
{
    // Forward declarations.
    class ProcessSharedAttributeOwner;
    class SharedAttribute;


    /**
     * Class description goes here
     */
    class DrawingDemonstration : public Process
    {
    public:
        // wxWidgets macro for dynamic by-name instantiation
        DECLARE_DYNAMIC_CLASS(DrawingDemonstration)

        // this is rootmap's macro to force this class to be linked into the executable
        // (otherwise the linker strips it out because there is no explicit use of the class)
        DECLARE_DYNAMIC_CLASS_FORCE_USE(DrawingDemonstration)

    public:
        DrawingDemonstration();
        ~DrawingDemonstration();

        /**
         *
         */
        virtual long int Initialise(ProcessActionDescriptor* action);

        /**
         *
         */
        virtual long int DoNormalWakeUp(ProcessActionDescriptor* action);

        /**
         *
         */
        virtual bool DoesOverride() const;

        // Drawing
        virtual bool DoesDrawing() const;
        virtual void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard);


    private:
        RootMapLoggerDeclaration();

        /**
         * Adds another line to the set and draws it
         */
        void DrawAnotherLine(ProcessActionDescriptor* action);

        //
        // SharedAttribute support
        //

        /**
         */
        ProcessSharedAttributeOwner* mySharedAttributeOwner;

        /**
         */
        void PrepareSharedAttributes(ProcessActionDescriptor* action);

        //
        // Data Members
        //

        /**
         */
        SharedAttribute* myLineFrequency;

        /**
         */
        typedef std::pair<DoubleCoordinate, DoubleCoordinate> Line3D;

        /**
         */
        typedef std::set<Line3D> LineSet;

        /**
         */
        LineSet myLines;
    }; // class DrawingDemonstration
} /* namespace rootmap */

#endif // #ifndef DrawingDemonstration_H
