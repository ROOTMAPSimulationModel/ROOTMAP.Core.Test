/////////////////////////////////////////////////////////////////////////////
// Name:        DummyPlantTypeDA.cpp
// Purpose:     Definition of the DummyPlantTypeDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyPlantTypeDA.h"


namespace rootmap
{
    std::string DummyPlantTypeDA::getName() const
    {
        return m_name;
    }


    double DummyPlantTypeDA::getRootsToFoliageRatio() const
    {
        return 1.0;
    }


    double DummyPlantTypeDA::getVegetateToReproduceRatio() const
    {
        return 1.0;
    }


    double DummyPlantTypeDA::getStructureToPhotosynthesizeRatio() const
    {
        return 1.0;
    }


    double DummyPlantTypeDA::getInitialSeminalDeflection() const
    {
        return 1.0;
    }


    double DummyPlantTypeDA::getFirstSeminalProbability() const
    {
        return 1.0;
    }


    double DummyPlantTypeDA::getGerminationLag() const
    {
        return 1.0;
    }


    double DummyPlantTypeDA::getTemperatureOfZeroGrowth() const
    {
        return 7.0;
    }


    DummyPlantTypeDA::~DummyPlantTypeDA()
    {
    }


    DummyPlantTypeDA::DummyPlantTypeDA(const std::string& name)
        : m_name(name)
    {
    }
} /* namespace rootmap */

