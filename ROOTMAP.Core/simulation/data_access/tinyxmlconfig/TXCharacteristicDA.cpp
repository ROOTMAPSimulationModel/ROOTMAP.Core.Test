/////////////////////////////////////////////////////////////////////////////
// Name:        TXCharacteristicDA.h
// Purpose:     Declaration of the TXCharacteristicDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXCharacteristicDA.h"


namespace rootmap
{
    CharacteristicIdentifier TXCharacteristicDA::getIdentifier() const
    {
        return m_id;
    }


    std::string TXCharacteristicDA::getName() const
    {
        return m_name;
    }

    void TXCharacteristicDA::setName(std::string newName)
    {
        m_name = newName;
    }


    std::string TXCharacteristicDA::getUnits() const
    {
        return m_units;
    }


    ScoreboardStratum TXCharacteristicDA::getStratum() const
    {
        return m_stratum;
    }


    double TXCharacteristicDA::getMinimum() const
    {
        return m_minimum;
    }


    double TXCharacteristicDA::getMaximum() const
    {
        return m_maximum;
    }


    double TXCharacteristicDA::getDefault() const
    {
        return m_default;
    }


    bool TXCharacteristicDA::isVisible() const
    {
        return m_isVisible;
    }


    bool TXCharacteristicDA::isEdittable() const
    {
        return m_isEdittable;
    }


    bool TXCharacteristicDA::isSavable() const
    {
        return m_isSavable;
    }


    bool TXCharacteristicDA::hasSpecialPerBoxInfo() const
    {
        return m_hasSpecialPerBoxInfo;
    }

    bool TXCharacteristicDA::isValid() const
    {
        if (m_id < 0) return false;
        if (m_name.empty()) return false;
        if (m_units.empty()) return false;
        if (m_stratum == ScoreboardStratum::NONE) return false;
        // there aren't really any invalid values for these:
        //double m_minimum;
        //double m_maximum;
        //double m_default;
        //bool m_isVisible;
        //bool m_isEdittable;
        //bool m_isSavable;
        //bool m_hasSpecialPerBoxInfo;

        return true;
    }


    TXCharacteristicDA::~TXCharacteristicDA()
    {
    }

    TXCharacteristicDA::TXCharacteristicDA
    (CharacteristicIdentifier id,
        std::string& name,
        std::string& units,
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
