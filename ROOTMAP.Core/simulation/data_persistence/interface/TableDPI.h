#ifndef TableDPI_H
#define TableDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TableDPI.h
// Purpose:     Declaration of the TableDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/common/Structures.h"
#include "simulation/common/Types.h"

namespace rootmap
{
    class ScoreboardStratum;

    class TableDPI
    {
    public:
        /**
         * @return the process name
         */
        virtual void storeProcessName(const std::string& pname) = 0;

        /**
         * retrieves the characteristic name
         */
        virtual void storeCharacteristicName(const std::string& cname) = 0;

        /**
         * retrieves the view direction
         */
        virtual void storeViewDirection(ViewDirection viewDir) = 0;

        /**
         * retrieves the view direction
         */
        virtual void storeLayer(long layer) = 0;

        /**
         * retrieves the scoreboard stratum
         */
        virtual void storeScoreboardStratum(const ScoreboardStratum& stratum) = 0;

        /**
         * Virtual destructor, like all good base class destructors should be
         */
        virtual ~TableDPI()
        {
        }

    protected:
        TableDPI()
        {
        }
    }; // class TableDPI
} /* namespace rootmap */

#endif // #ifndef TableDPI_H
