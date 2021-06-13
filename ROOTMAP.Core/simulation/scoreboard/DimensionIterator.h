#ifndef DimensionIterator_H
#define DimensionIterator_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DimensionIterator.h
// Purpose:     Declaration of the DimensionIterator class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// Declaration of the DimensionIterator class
/////////////////////////////////////////////////////////////////////////////
#include "simulation/scoreboard/DimensionOrder.h"
#include "core/scoreboard/ScoreboardBox.h"

namespace rootmap
{
    class Scoreboard;

    /**
     * Encapsulates functionality for incrementing through 3 dimensions in an
     * arbitrarily specified order and direction. The ordering and direction
     * are specificied in the DimensionOrder class
     */
    struct DimensionIterator
    {
    public:
        DimensionIterator(DimensionOrder& dim_order, Scoreboard* scoreboard, BoxCoordinate& box_coord);

        virtual ~DimensionIterator();

        /** Initialise the outer dimension loop index */
        void outer_init();
        /** Initialise the middle dimension loop index */
        void middle_init();
        /** Initialise the inner dimension loop index */
        void inner_init();

        /** Test outer dimension loop continuation */
        bool outer_do_continue();
        /** Test outer dimension loop continuation */
        bool middle_do_continue();
        /** Test inner dimension loop continuation */
        bool inner_do_continue();

        /** "Increment" the outer dimension loop index */
        void outer_next();
        /** "Increment" the middle dimension loop index */
        void middle_next();
        /** "Increment" the inner dimension loop index */
        void inner_next();

    private:
        void SetStartEndMinMax(int index);

        /**
         * Specifies the order and direction of iteration
         */
        DimensionOrder m_dimOrder;

        /**
         * Contains the max and min x,y,z indices
         */
        Scoreboard* m_scoreboard;

        /**
         *
         */
        BoxCoordinate& m_boxCoord;

        /**
         *
         */
        BoxCoordinateIndex m_start[3];

        /**
         *
         */
        BoxCoordinateIndex m_end[3];
    }; // class DimensionIterator
} /* namespace rootmap */

#endif // #ifndef DimensionIterator_H
