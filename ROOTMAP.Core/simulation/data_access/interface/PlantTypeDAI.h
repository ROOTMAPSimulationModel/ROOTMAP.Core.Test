#ifndef PlantTypeDAI_H
#define PlantTypeDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantTypeDAI.h
// Purpose:     Declaration of the PlantTypeDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Types.h"


namespace rootmap
{
    class PlantTypeDAI
    {
    public:
        virtual std::string getName() const = 0;

        virtual double getRootsToFoliageRatio() const = 0;
        virtual double getVegetateToReproduceRatio() const = 0;
        virtual double getStructureToPhotosynthesizeRatio() const = 0;
        virtual double getInitialSeminalDeflection() const = 0;
        virtual double getFirstSeminalProbability() const = 0;
        virtual double getGerminationLag() const = 0;
        virtual double getTemperatureOfZeroGrowth() const = 0;

        virtual ~PlantTypeDAI()
        {
        }

    protected:
        PlantTypeDAI()
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
    }; // class PlantTypeDAI
} /* namespace rootmap */

#endif // #ifndef PlantTypeDAI_H
