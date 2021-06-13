/////////////////////////////////////////////////////////////////////////////
// Name:        PlantDataPreviewer.cpp
// Purpose:     Implementation of the PlantDataPreviewer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/PlantDataPreviewer.h"
#include "simulation/process/plant/PlantType.h"

namespace rootmap
{
    PlantDataPreviewer::PlantDataPreviewer()
        : m_possibleSeminals(__nullptr)
        , m_possibleNodals(__nullptr)
        , m_nextPlantIndex(0)
    {
    }

    PlantDataPreviewer::~PlantDataPreviewer()
    {
        for (std::vector<TogglablePlantDAI*>::iterator iter = m_plants.begin(); iter != m_plants.end(); ++iter)
        {
            delete *iter;
        }
        for (std::vector<PlantType*>::iterator iter = m_plantTypes.begin(); iter != m_plantTypes.end(); ++iter)
        {
            delete *iter;
        }
    }

    void PlantDataPreviewer::AddPlant(TogglablePlantDAI* plantData)
    {
        m_plants.push_back(plantData);
    }

    void PlantDataPreviewer::AddPlantType(PlantType* plantType)
    {
        m_plantTypes.push_back(plantType);
    }

    TogglablePlantDAI* PlantDataPreviewer::GetNextPlant()
    {
        if (m_nextPlantIndex < m_plants.size())
        {
            return m_plants[m_nextPlantIndex++];
        }
        return __nullptr;
    }

    void PlantDataPreviewer::ReplacePlant(TogglablePlantDAI* oldPlant, TogglablePlantDAI* newPlant)
    {
        for (std::vector<TogglablePlantDAI*>::iterator iter = m_plants.begin(); iter != m_plants.end(); ++iter)
        {
            if ((*iter) == oldPlant)
            {
                delete oldPlant;
                (*iter) = newPlant;
                return;
            }
        }
    }
} /* namespace rootmap */

