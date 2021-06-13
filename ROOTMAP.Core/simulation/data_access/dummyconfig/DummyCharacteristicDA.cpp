/////////////////////////////////////////////////////////////////////////////
// Name:        DummyCharacteristicDA.h
// Purpose:     Declaration of the DummyCharacteristicDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyCharacteristicDA.h"


namespace rootmap
{
    CharacteristicIdentifier DummyCharacteristicDA::getIdentifier() const
    {
        return m_id;
    }


    std::string DummyCharacteristicDA::getName() const
    {
        return m_name;
    }


    std::string DummyCharacteristicDA::getUnits() const
    {
        return m_units;
    }


    ScoreboardStratum DummyCharacteristicDA::getStratum() const
    {
        return m_stratum;
    }


    double DummyCharacteristicDA::getMinimum() const
    {
        return m_minimum;
    }


    double DummyCharacteristicDA::getMaximum() const
    {
        return m_maximum;
    }


    double DummyCharacteristicDA::getDefault() const
    {
        return m_default;
    }


    bool DummyCharacteristicDA::isVisible() const
    {
        return m_isVisible;
    }


    bool DummyCharacteristicDA::isEdittable() const
    {
        return m_isEdittable;
    }


    bool DummyCharacteristicDA::isSavable() const
    {
        return m_isSavable;
    }


    bool DummyCharacteristicDA::hasSpecialPerBoxInfo() const
    {
        return m_hasSpecialPerBoxInfo;
    }


    DummyCharacteristicDA::~DummyCharacteristicDA()
    {
    }

    DummyCharacteristicDA::DummyCharacteristicDA
    (CharacteristicIdentifier id,
        const std::string& name,
        const std::string& units,
        ScoreboardStratum stratum,
        double minimum,
        double maximum,
        double default_,
        bool isVisible,
        bool isEdittable,
        bool isSavable,
        bool hasSpecialPerBoxInfo)
        : m_id(id)
        , m_name(name)
        , m_units(units)
        , m_stratum(stratum)
        , m_minimum(minimum)
        , m_maximum(maximum)
        , m_default(default_)
        , m_isVisible(isVisible)
        , m_isEdittable(isEdittable)
        , m_isSavable(isSavable)
        , m_hasSpecialPerBoxInfo(hasSpecialPerBoxInfo)
    {
    }
} /* namespace rootmap */

