/////////////////////////////////////////////////////////////////////////////
// Name:        DummyProcessDA.cpp
// Purpose:     Definition of the DummyProcessDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyProcessDA.h"
#include "simulation/data_access/dummyconfig/DummyCharacteristicDA.h"

#include "core/common/RmAssert.h"


namespace rootmap
{
    ProcessIdentifier DummyProcessDA::getIdentifier() const
    {
        return m_identifier;
    }


    std::string DummyProcessDA::getName() const
    {
        return m_name;
    }


    ScoreboardStratum DummyProcessDA::getStratum() const
    {
        return m_stratum;
    }


    ProcessActivity DummyProcessDA::getActivity() const
    {
        return m_activity;
    }

    const CharacteristicDAICollection& DummyProcessDA::getCharacteristics() const
    {
        return m_characteristics;
    }

    bool DummyProcessDA::doesOverride() const
    {
        return m_doesOverride;
    }

    void DummyProcessDA::addCharacteristicDA(DummyCharacteristicDA* characteristicDA)
    {
        RmAssert(__nullptr != characteristicDA, "NULL characteristic data given to addCharacteristicDA()");

        m_characteristics.push_back(characteristicDA);
    }

    DummyProcessDA::~DummyProcessDA()
    {
    }


    DummyProcessDA::DummyProcessDA
    (const std::string& name,
        bool doesOverride,
        ProcessIdentifier identifier,
        ScoreboardStratum stratum,
        ProcessActivity activity)
        : m_name(name)
        , m_identifier(identifier)
        , m_stratum(stratum)
        , m_activity(activity)
        , m_doesOverride(doesOverride)
    {
    }
} /* namespace rootmap */

