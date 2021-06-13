/////////////////////////////////////////////////////////////////////////////
// Name:        DummyPlantSummaryDA.cpp
// Purpose:     Definition of the DummyPlantSummaryDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyPlantSummaryDA.h"


namespace rootmap
{
    DummyPlantSummaryDA::DummyPlantSummaryDA
    (const std::string& name,
        const ScoreboardStratum& stratum,
        PlantSummaryIdentifier identifier,
        WrapDirection wrap_direction,
        SummaryRoutine summary_type,
        const std::vector<std::string>& plants)
        : m_name(name)
        , m_stratum(stratum)
        , m_identifier(identifier)
        , m_wrapDirection(wrap_direction)
        , m_summaryType(summary_type)
        , m_plants(plants)
    {
    }


    DummyPlantSummaryDA::~DummyPlantSummaryDA()
    {
    }


    std::string DummyPlantSummaryDA::getName() const
    {
        return m_name;
    }


    ScoreboardStratum DummyPlantSummaryDA::getStratum() const
    {
        return m_stratum;
    }


    PlantSummaryIdentifier DummyPlantSummaryDA::getIdentifier() const
    {
        // IdentifierUtility::useNextPlantSummaryIdentifier(getStratum() const);
        return m_identifier;
    }


    WrapDirection DummyPlantSummaryDA::getWrapDirection() const
    {
        return m_wrapDirection;
    }


    SummaryRoutine DummyPlantSummaryDA::getSummaryType() const
    {
        return m_summaryType;
    }


    std::vector<std::string> DummyPlantSummaryDA::getPlants() const
    {
        return m_plants;
    }
} /* namespace rootmap */

