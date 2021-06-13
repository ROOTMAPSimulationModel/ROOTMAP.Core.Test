#ifndef PlantTypeDPI_H
#define PlantTypeDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantTypeDPI.h
// Purpose:     Declaration of the PlantTypeDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Types.h"


namespace rootmap
{
    class PlantTypeDPI
    {
    public:
        virtual void storeName(const std::string& name) = 0;

        virtual void storeRootsToFoliageRatio(double value) = 0;
        virtual void storeVegetateToReproduceRatio(double value) = 0;
        virtual void storeStructureToPhotosynthesizeRatio(double value) = 0;
        virtual void storeInitialSeminalDeflection(double value) = 0;
        virtual void storeFirstSeminalProbability(double value) = 0;
        virtual void storeGerminationLag(double value) = 0;
        virtual void storeTemperatureOfZeroGrowth(double value) = 0;

        virtual ~PlantTypeDPI()
        {
        }

    protected:
        PlantTypeDPI()
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
    }; // class PlantTypeDPI
} /* namespace rootmap */

#endif // #ifndef PlantTypeDPI_H
