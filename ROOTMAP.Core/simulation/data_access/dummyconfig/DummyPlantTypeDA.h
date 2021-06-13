#ifndef DummyPlantTypeDA_H
#define DummyPlantTypeDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyPlantTypeDA.h
// Purpose:     Declaration of the DummyPlantTypeDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/PlantTypeDAI.h"

namespace rootmap
{
    class DummyPlantTypeDA : public PlantTypeDAI
    {
    public:
        virtual std::string getName() const;

        virtual double getRootsToFoliageRatio() const;
        virtual double getVegetateToReproduceRatio() const;
        virtual double getStructureToPhotosynthesizeRatio() const;
        virtual double getInitialSeminalDeflection() const;
        virtual double getFirstSeminalProbability() const;
        virtual double getGerminationLag() const;
        virtual double getTemperatureOfZeroGrowth() const;

        virtual ~DummyPlantTypeDA();

        DummyPlantTypeDA(const std::string& name);

    private:

        ///
        /// member declaration
        std::string m_name;
    }; // class DummyPlantTypeDA
} /* namespace rootmap */

#endif // #ifndef DummyPlantTypeDA_H
