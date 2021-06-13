#ifndef PlantDPI_H
#define PlantDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantDPI.h
// Purpose:     Declaration of the PlantDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/common/DoubleCoordinates.h"

#include "simulation/common/Types.h"
#include "simulation/data_access/interface/ProcessDAI.h"

namespace rootmap
{
    class PossibleAxis;

    class PlantDPI : public virtual ProcessDAI
    {
    public:
        /**
         * retrieves the name of the PlantType
         */
        virtual storeTypeName(std::string& typeName) = 0;

        virtual storeOrigin(DoubleCoordinate origin) = 0;
        virtual storeSeedLocation(DoubleCoordinate seedLocation) = 0;
        virtual storeSeedingTime(ProcessTime_t seedingTime) = 0;

        virtual storePossibleSeminals(PossibleAxis* possibleSeminals) = 0;
        virtual storePossibleNodals(PossibleAxis* possibleNodals) = 0;

        virtual ~PlantDPI()
        {
        }

    protected:
        PlantDPI()
        {
        }

        /**
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */

    private:

        ///
        /// member declaration
    }; // class PlantDPI
} /* namespace rootmap */

#endif // #ifndef PlantDPI_H
