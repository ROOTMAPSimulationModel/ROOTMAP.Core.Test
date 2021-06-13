/////////////////////////////////////////////////////////////////////////////
// Name:        DimensionIterator.cpp
// Purpose:     Implementation of the DimensionIterator class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/scoreboard/DimensionIterator.h"
#include "simulation/scoreboard/Scoreboard.h"
#include "core/common/Exceptions.h"
#include "core/utility/Utility.h"


namespace rootmap
{
    DimensionIterator::DimensionIterator(DimensionOrder& dim_order, Scoreboard* scoreboard, BoxCoordinate& box_coord)
        : m_dimOrder(dim_order)
        , m_scoreboard(scoreboard)
        , m_boxCoord(box_coord)
        //, m_start(0,0,0)
        //, m_end(0,0,0)
    {
        SetStartEndMinMax(0);
        SetStartEndMinMax(1);
        SetStartEndMinMax(2);
    }

    DimensionIterator::~DimensionIterator()
    {
    }

    /** Initialise the outer dimension loop index */
    void DimensionIterator::outer_init()
    {
        switch (m_dimOrder.m_dimensions[2])
        {
        case X: m_boxCoord.x = m_start[2];
            break;
        case Y: m_boxCoord.y = m_start[2];
            break;
        case Z: m_boxCoord.z = m_start[2];
            break;
        default:
            throw new RmException("Bad dimension specified in DimensionOrder");
            break;
        }
    }

    /** Initialise the middle dimension loop index */
    void DimensionIterator::middle_init()
    {
        switch (m_dimOrder.m_dimensions[1])
        {
        case X: m_boxCoord.x = m_start[1];
            break;
        case Y: m_boxCoord.y = m_start[1];
            break;
        case Z: m_boxCoord.z = m_start[1];
            break;
        default:
            throw new RmException("Bad dimension specified in DimensionOrder");
            break;
        }
    }

    /** Initialise the inner dimension loop index */
    void DimensionIterator::inner_init()
    {
        switch (m_dimOrder.m_dimensions[0])
        {
        case X: m_boxCoord.x = m_start[0];
            break;
        case Y: m_boxCoord.y = m_start[0];
            break;
        case Z: m_boxCoord.z = m_start[0];
            break;
        default:
            throw new RmException("Bad dimension specified in DimensionOrder");
            break;
        }
    }


    /** Test outer dimension loop continuation */
    bool DimensionIterator::outer_do_continue()
    {
        if (m_dimOrder.m_increments[2] > 0)
        {
            switch (m_dimOrder.m_dimensions[2])
            {
            case X: return (m_boxCoord.x <= m_end[2]);
            case Y: return (m_boxCoord.y <= m_end[2]);
            case Z: return (m_boxCoord.z <= m_end[2]);
            default:
                throw new RmException("Bad dimension specified in DimensionOrder");
                break;
            }
        }
        else // negative increment - check for >= end
        {
            switch (m_dimOrder.m_dimensions[2])
            {
            case X: return (m_boxCoord.x >= m_end[2]);
            case Y: return (m_boxCoord.y >= m_end[2]);
            case Z: return (m_boxCoord.z >= m_end[2]);
            default:
                throw new RmException("Bad dimension specified in DimensionOrder");
                break;
            }
        }
    }

    /** Test outer dimension loop continuation */
    bool DimensionIterator::middle_do_continue()
    {
        if (m_dimOrder.m_increments[1] > 0)
        {
            switch (m_dimOrder.m_dimensions[1])
            {
            case X: return (m_boxCoord.x <= m_end[1]);
            case Y: return (m_boxCoord.y <= m_end[1]);
            case Z: return (m_boxCoord.z <= m_end[1]);
            default:
                throw new RmException("Bad dimension specified in DimensionOrder");
                break;
            }
        }
        else // negative increment - check for >= end
        {
            switch (m_dimOrder.m_dimensions[1])
            {
            case X: return (m_boxCoord.x >= m_end[1]);
            case Y: return (m_boxCoord.y >= m_end[1]);
            case Z: return (m_boxCoord.z >= m_end[1]);
            default:
                throw new RmException("Bad dimension specified in DimensionOrder");
                break;
            }
        }
    }

    /** Test inner dimension loop continuation */
    bool DimensionIterator::inner_do_continue()
    {
        if (m_dimOrder.m_increments[0] > 0)
        {
            switch (m_dimOrder.m_dimensions[0])
            {
            case X: return (m_boxCoord.x <= m_end[0]);
            case Y: return (m_boxCoord.y <= m_end[0]);
            case Z: return (m_boxCoord.z <= m_end[0]);
            default:
                throw new RmException("Bad dimension specified in DimensionOrder");
                break;
            }
        }
        else // negative increment - check for >= end
        {
            switch (m_dimOrder.m_dimensions[0])
            {
            case X: return (m_boxCoord.x >= m_end[0]);
            case Y: return (m_boxCoord.y >= m_end[0]);
            case Z: return (m_boxCoord.z >= m_end[0]);
            default:
                throw new RmException("Bad dimension specified in DimensionOrder");
                break;
            }
        }
    }


    /** "Increment" the outer dimension loop index */
    void DimensionIterator::outer_next()
    {
        switch (m_dimOrder.m_dimensions[2])
        {
        case X: m_boxCoord.x += m_dimOrder.m_increments[2];
            break;
        case Y: m_boxCoord.y += m_dimOrder.m_increments[2];
            break;
        case Z: m_boxCoord.z += m_dimOrder.m_increments[2];
            break;
        default:
            throw new RmException("Bad dimension specified in DimensionOrder");
            break;
        }
    }

    /** "Increment" the middle dimension loop index */
    void DimensionIterator::middle_next()
    {
        switch (m_dimOrder.m_dimensions[1])
        {
        case X: m_boxCoord.x += m_dimOrder.m_increments[1];
            break;
        case Y: m_boxCoord.y += m_dimOrder.m_increments[1];
            break;
        case Z: m_boxCoord.z += m_dimOrder.m_increments[1];
            break;
        default:
            throw new RmException("Bad dimension specified in DimensionOrder");
            break;
        }
    }

    /** "Increment" the inner dimension loop index */
    void DimensionIterator::inner_next()
    {
        switch (m_dimOrder.m_dimensions[0])
        {
        case X: m_boxCoord.x += m_dimOrder.m_increments[0];
            break;
        case Y: m_boxCoord.y += m_dimOrder.m_increments[0];
            break;
        case Z: m_boxCoord.z += m_dimOrder.m_increments[0];
            break;
        default:
            throw new RmException("Bad dimension specified in DimensionOrder");
            break;
        }
    }

    void DimensionIterator::SetStartEndMinMax(int index)
    {
        BoxCoordinateIndex max_x, max_y, max_z;
        m_scoreboard->GetNumLayers(max_x, max_y, max_z);

        // reserved for future
        // m_scoreboard->GetBoxExtents(blah);
        BoxCoordinateIndex min_x = 1, min_y = 1, min_z = 1;

        switch (m_dimOrder.m_dimensions[index])
        {
        case X:
            if (m_dimOrder.m_increments[index] > 0)
            {
                m_start[index] = min_x;
                m_end[index] = max_x;
            }
            else
            {
                m_start[index] = max_x;
                m_end[index] = min_x;
            }
            break;
        case Y:
            if (m_dimOrder.m_increments[index] > 0)
            {
                m_start[index] = min_y;
                m_end[index] = max_y;
            }
            else
            {
                m_start[index] = max_y;
                m_end[index] = min_y;
            }
            break;
        case Z:
            if (m_dimOrder.m_increments[index] > 0)
            {
                m_start[index] = min_z;
                m_end[index] = max_z;
            }
            else
            {
                m_start[index] = max_z;
                m_end[index] = min_z;
            }
            break;
        default:
            throw new RmException("Bad dimension specified in DimensionOrder");
            break;
        }
    } // SetStartEndMinMax(int index)
} /* namespace rootmap */

