#ifndef PlantDAI_H
#define PlantDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantDAI.h
// Purpose:     Declaration of the PlantDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/common/DoubleCoordinates.h"

#include "simulation/common/Types.h"
#include "simulation/process/plant/PossibleAxis.h"
#include "simulation/data_access/interface/ProcessDAI.h"

namespace rootmap
{
    class PlantDAI : public virtual ProcessDAI
    {
    public:
        /**
         * retrieves the name of the PlantType
         */
        virtual std::string getTypeName() const = 0;

        virtual DoubleCoordinate getOrigin() const = 0;
        virtual DoubleCoordinate getSeedLocation() const = 0;
        virtual ProcessTime_t getSeedingTime() const = 0;

        virtual PossibleAxis* getPossibleSeminals(bool relinquish = true) const = 0;
        virtual PossibleAxis* getPossibleNodals(bool relinquish = true) const = 0;

        virtual ~PlantDAI()
        {
        }

    protected:
        PlantDAI()
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
    }; // class PlantDAI
} /* namespace rootmap */

#endif // #ifndef PlantDAI_H
