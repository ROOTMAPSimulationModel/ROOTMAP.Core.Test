/////////////////////////////////////////////////////////////////////////////
// Name:        DrawingDemonstration.cpp
// Purpose:     Implementation of the DrawingDemonstration class
// Created:     10-07-2009
// Author:      RvH
// $Date$
// $Revision$
// Copyright:   ©2002-2009 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

// Typically, your Process derived class will be in the following location
#include "simulation/process/modules/DrawingDemonstration.h"

// Required if drawing
#include "simulation/process/common/ProcessDrawing.h"
#include "simulation/process/common/Process_Dictionary.h"

// Shared Attributes - read the doco
#include "simulation/process/shared_attributes/ProcessSharedAttributeOwner.h"
#include "simulation/process/shared_attributes/ProcessAttributeSupplier.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeSearchHelper.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"


// Useful macro, example:
// LOG_ERROR_IF(value<0);
#define LOG_ERROR_IF(s) if ( (s) ) { RootMapLogError(LOG_LINE "Encountered error in logic : " #s); }


namespace rootmap
{
    // Put constants here as const variables, not preprocessor macros
    const double SomeConstant = 1.00;
    const int SomeOtherInteger = 1;

    // Causes the logger static var to be initialised
    RootMapLoggerDefinition(DrawingDemonstration);

    // wxWidgets stuff, so the process can be dynamically instantiated by name
    IMPLEMENT_DYNAMIC_CLASS(DrawingDemonstration, Process)

        // Oh hai, i'm in ur constructor
        DrawingDemonstration::DrawingDemonstration()
        : mySharedAttributeOwner(__nullptr)
    {
        RootMapLoggerInitialisation("rootmap.DrawingDemonstration");

        // Here is where your attribute owner is initialised
        mySharedAttributeOwner = new ProcessSharedAttributeOwner("DrawingDemonstration", this);
        SharedAttributeRegistrar::RegisterOwner(mySharedAttributeOwner);

        // And the supplier of any attributes you may have
        ProcessAttributeSupplier* supplier = new ProcessAttributeSupplier(this, "DrawingDemonstration");
        SharedAttributeRegistrar::RegisterSupplier(supplier);
    }

    DrawingDemonstration::~DrawingDemonstration()
    {
    }


    long int DrawingDemonstration::Initialise(ProcessActionDescriptor* action)
    {
        Use_PostOffice;
        Use_ReturnValue;

        // our shared attribute determines our wakeup time
        PrepareSharedAttributes(action);

        ProcessTime_t time_between_waking = PostOffice::CalculateTimeSpan(0, 0, 1, 0, 0, 0) / (myLineFrequency->GetValue());
        SetPeriodicDelay(kNormalMessage, time_between_waking);
        Send_GeneralPurpose(action->GetTime() + time_between_waking, this, __nullptr, kNormalMessage);

        // allows us to track how long it has been since the last message
        SetTimeDeltaTracking(kNormalMessage);
        //SetTimeDeltaTracking(kGeneralPurposeMessage0);

        Process::Initialise(action);

        return (return_value);
    } // long int DrawingDemonstration::Initialise(ProcessActionDescriptor *action)


    void DrawingDemonstration::PrepareSharedAttributes(ProcessActionDescriptor* action)
    {
        SharedAttributeSearchHelper helper(action->GetSharedAttributeManager(), mySharedAttributeOwner);

        myLineFrequency = helper.SearchForAttribute("Line Frequency");
    }


    long int DrawingDemonstration::DoNormalWakeUp(ProcessActionDescriptor* action)
    {
        DrawAnotherLine(action);

        return (Process::DoNormalWakeUp(action));
    }

    void DrawingDemonstration::DrawAnotherLine(ProcessActionDescriptor* action)
    {
        Use_ScoreboardCoordinator;
        Use_Scoreboard;
        const ScoreboardStratum& stratum = scoreboard->GetScoreboardStratum();

        /*
        Draw lines according to following test scenarios:

        Line entirely within main scoreboard, oriented within 15-30degrees of the X axis
        Line entirely within main scoreboard, oriented within 15-30degrees of the Y axis
        Line entirely within main scoreboard, oriented within 15-30degrees of the Z axis

        Line crosses main to adjacent scoreboard, across X-minimum boundary
        Line crosses main to adjacent scoreboard, across X-maximum boundary
        Line crosses main to adjacent scoreboard, across Y-minimum boundary
        Line crosses main to adjacent scoreboard, across Y-maximum boundary
        Line crosses main to above-scoreboard, across Z-minimum boundary
        Line crosses main to below-scoreboard, across Z-maximum boundary

        Line entirely within adjacent (to X-min) scoreboard, oriented within 15-30degrees of the X axis
        Line entirely within adjacent (to X-min) scoreboard, oriented within 15-30degrees of the Y axis
        Line entirely within adjacent (to X-min) scoreboard, oriented within 15-30degrees of the Z axis
        Line entirely within adjacent (to X-max) scoreboard, oriented within 15-30degrees of the X axis
        Line entirely within adjacent (to X-max) scoreboard, oriented within 15-30degrees of the Y axis
        Line entirely within adjacent (to X-max) scoreboard, oriented within 15-30degrees of the Z axis

        Line entirely within adjacent (to Y-min) scoreboard, oriented within 15-30degrees of the X axis
        Line entirely within adjacent (to Y-min) scoreboard, oriented within 15-30degrees of the Y axis
        Line entirely within adjacent (to Y-min) scoreboard, oriented within 15-30degrees of the Z axis
        Line entirely within adjacent (to Y-max) scoreboard, oriented within 15-30degrees of the X axis
        Line entirely within adjacent (to Y-max) scoreboard, oriented within 15-30degrees of the Y axis
        Line entirely within adjacent (to Y-max) scoreboard, oriented within 15-30degrees of the Z axis

        Line entirely below scoreboard (>Z-max), oriented within 15-30degrees of the X axis
        Line entirely above scoreboard (<Z-min), oriented within 15-30degrees of the Y axis
        Line entirely below scoreboard (>Z-max), oriented within 15-30degrees of the Z axis
        Line entirely above scoreboard (<Z-min), oriented within 15-30degrees of the X axis
        Line entirely below scoreboard (>Z-max), oriented within 15-30degrees of the Y axis
        Line entirely above scoreboard (<Z-min), oriented within 15-30degrees of the Z axis

        Line crosses adjacent to next-adjacent scoreboard, across X-minimum boundary
        Line crosses adjacent to next-adjacent scoreboard, across X-maximum boundary
        Line crosses adjacent to next-adjacent scoreboard, across Y-minimum boundary
        Line crosses adjacent to next-adjacent scoreboard, across Y-maximum boundary


        others to test:
        Line crosses both X&Y scoreboard boundary, ie. to the diagonally-adjacent scoreboard
        Polygons (filled and non- )

        */

        if (myLines.size() < 16)
        {
            DoubleBox scoreboard_bounds;
            scoreboardcoordinator->GetScoreboardBounds(stratum, scoreboard_bounds);

            switch (myLines.size())
            {
            case 0:
                // Line entirely within main scoreboard, oriented within 15-30degrees of the X axis
                break;
            case 1:
                // Line entirely within main scoreboard, oriented within 15-30degrees of the Y axis
                break;
            case 2:
                // Line entirely within main scoreboard, oriented within 15-30degrees of the Z axis
                break;
            case 3:
                //Line crosses main to adjacent scoreboard, across X-minimum boundary
                break;
            case 4:
                //Line crosses main to adjacent scoreboard, across X-maximum boundary
                break;
            case 5:
                //Line crosses main to adjacent scoreboard, across Y-minimum boundary
                break;
            case 6:
                //Line crosses main to adjacent scoreboard, across Y-maximum boundary
                break;
            case 7:
                //Line crosses main to above-scoreboard, across Z-minimum boundary
                break;
            case 8:
                //Line crosses main to below-scoreboard, across Z-maximum boundary
                break;
            case 9:
                //Line entirely within adjacent (to X-min) scoreboard, oriented within 15-30degrees of the X axis
                break;
            case 10:
                //Line entirely within adjacent (to X-min) scoreboard, oriented within 15-30degrees of the Y axis
                break;
            case 11:
                //Line entirely within adjacent (to X-min) scoreboard, oriented within 15-30degrees of the Z axis
                break;
            case 12:
                //Line entirely within adjacent (to X-max) scoreboard, oriented within 15-30degrees of the X axis
                break;
            case 13:
                //Line entirely within adjacent (to X-max) scoreboard, oriented within 15-30degrees of the Y axis
                break;
            case 14:
                //Line entirely within adjacent (to X-max) scoreboard, oriented within 15-30degrees of the Z axis
                break;
            case 15:
                //Line entirely within adjacent (to Y-min) scoreboard, oriented within 15-30degrees of the X axis
                break;
            case 16:
                //Line entirely within adjacent (to Y-min) scoreboard, oriented within 15-30degrees of the Y axis
                break;
            case 17:
                //Line entirely within adjacent (to Y-min) scoreboard, oriented within 15-30degrees of the Z axis
                break;
            case 18:
                //Line entirely within adjacent (to Y-max) scoreboard, oriented within 15-30degrees of the X axis
                break;
            case 19:
                //Line entirely within adjacent (to Y-max) scoreboard, oriented within 15-30degrees of the Y axis
                break;
            case 20:
                //Line entirely within adjacent (to Y-max) scoreboard, oriented within 15-30degrees of the Z axis
                break;
            case 21:
                //Line entirely below scoreboard (>Z-max), oriented within 15-30degrees of the X axis
                break;
            case 22:
                //Line entirely above scoreboard (<Z-min), oriented within 15-30degrees of the Y axis
                break;
            case 23:
                //Line entirely below scoreboard (>Z-max), oriented within 15-30degrees of the Z axis
                break;
            case 24:
                //Line entirely above scoreboard (<Z-min), oriented within 15-30degrees of the X axis
                break;
            case 25:
                //Line entirely below scoreboard (>Z-max), oriented within 15-30degrees of the Y axis
                break;
            case 26:
                //Line entirely above scoreboard (<Z-min), oriented within 15-30degrees of the Z axis
                break;
            case 27:
                //Line crosses adjacent to next-adjacent scoreboard, across X-minimum boundary
                break;
            case 28:
                //Line crosses adjacent to next-adjacent scoreboard, across X-maximum boundary
                break;
            case 29:
                //Line crosses adjacent to next-adjacent scoreboard, across Y-minimum boundary
                break;
            case 30:
                //Line crosses adjacent to next-adjacent scoreboard, across Y-maximum boundary
                break;
            case 31:
                //
                break;
            case 32:
                //
                break;

            default:
                break;
            }
        }
    }

    void DrawingDemonstration::DrawScoreboard(const DoubleRect& /* area */, Scoreboard* /* scoreboard */)
    {
        BEGIN_DRAWING
            for (LineSet::iterator iter(myLines.begin());
                iter != myLines.end(); ++iter)
        {
            const DoubleCoordinate& coord1 = (*iter).first;
            const DoubleCoordinate& coord2 = (*iter).second;

            drawing.DrawLine(coord1, coord2);
        }
        END_DRAWING
    }


    bool DrawingDemonstration::DoesOverride() const
    {
        return (true);
    }

    bool DrawingDemonstration::DoesDrawing() const
    {
        return (true);
    }
} /* namespace rootmap */

