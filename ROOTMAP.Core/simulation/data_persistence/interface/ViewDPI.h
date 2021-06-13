#ifndef ViewDPI_H
#define ViewDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ViewDPI.h
// Purpose:     Declaration of the ViewDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/common/DoubleCoordinates.h"

#include "simulation/common/Types.h"
#include "simulation/scoreboard/ScoreboardFlags.h"
#include "simulation/data_access/interface/ViewDAI.h"

#include <vector>

namespace rootmap
{
    class Process;

    class ViewDPI
    {
    public:

        /**
         * @return
         */
        virtual void storeViewDirection(ViewDirection viewDir) = 0;

        /**
         * @return
         */
        virtual void storeReferenceIndex(DoubleCoordinate referenceIndex) = 0;

        /**
         * @return
         */
        virtual void storeScale(double scale) = 0;

        /**
         * @return
         */
        virtual void storeDoesRepeat(bool doesRepeat) = 0;

        /**
         * @return
         */
        virtual void storeDoesWrap(bool doesWrap) = 0;

        /**
         * @return
         */
        virtual void storeDoesBoxes(bool doesBoxes) = 0;

        /**
         * @return
         */
        virtual void storeDoesBoundaries(bool doesBoundaries) = 0;

        /**
         * @return
         */
        virtual void storeDoesBoxColours(bool doesBoxColours) = 0;

        /**
         * @return
         */
        virtual void storeScoreboards(const ScoreboardFlags& scoreboards) = 0;

        /**
         * @return
         */
        virtual void storeProcesses(const std::vector<std::string>& pnames) = 0;


        /**
         * @return
         */
        virtual void storeCharacteristicColourInfo(const ViewDAI::CharacteristicColourDataArray& colours) = 0;

        /**
         * Virtual destructor, like all good base class destructors should be
         */
        virtual ~ViewDPI()
        {
        }

    protected:
        ViewDPI()
        {
        }
    }; // class ViewDPI
} /* namespace rootmap */

#endif // #ifndef ViewDPI_H
