#ifndef PlantSummaryDAI_H
#define PlantSummaryDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantSummaryDAI.h
// Purpose:     Declaration of the PlantSummaryDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/scoreboard/ScoreboardStratum.h"
#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "simulation/common/Types.h"
#include <vector>

namespace rootmap
{
    class PlantSummaryDAI
    {
    public:
        virtual std::string getName() const = 0;
        virtual ScoreboardStratum getStratum() const = 0;
        virtual PlantSummaryIdentifier getIdentifier() const = 0;

        virtual WrapDirection getWrapDirection() const = 0;
        virtual SummaryRoutine getSummaryType() const = 0;

        virtual std::vector<std::string> getPlants() const = 0;

        virtual ~PlantSummaryDAI()
        {
        }

    protected:
        PlantSummaryDAI()
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
    }; // class PlantSummaryDAI
} /* namespace rootmap */

#endif // #ifndef PlantSummaryDAI_H
