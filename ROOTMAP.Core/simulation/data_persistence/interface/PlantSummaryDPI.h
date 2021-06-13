#ifndef PlantSummaryDPI_H
#define PlantSummaryDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantSummaryDPI.h
// Purpose:     Declaration of the PlantSummaryDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

//#include "core/scoreboard/ScoreboardStratum.h"
#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "simulation/common/Types.h"
#include <vector>

namespace rootmap
{
    class ScoreboardStratum;

    class PlantSummaryDPI
    {
    public:
        virtual storeName(const std::string& name) = 0;
        virtual storeStratum(const ScoreboardStratum& stratum) = 0;
        virtual storeIdentifier(const PlantSummaryIdentifier id) = 0;

        virtual storeWrapDirection(WrapDirection viewDir) = 0;
        virtual storeSummaryType(SummaryRoutine summRoutine) = 0;

        virtual storePlants(const std::vector<std::string>& plantNames) = 0;

        virtual ~PlantSummaryDPI()
        {
        }

    protected:
        PlantSummaryDPI()
        {
        }

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */

    private:

        ///
        /// member declaration
    }; // class PlantSummaryDPI
} /* namespace rootmap */

#endif // #ifndef PlantSummaryDPI_H
