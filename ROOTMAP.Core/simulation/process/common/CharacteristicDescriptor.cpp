/////////////////////////////////////////////////////////////////////////////
// Name:        CharacteristicDescriptor.cpp
// Purpose:     Implementation of the CharacteristicDescriptor class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-11-06 00:38:15 +0900 (Thu, 06 Nov 2008) $
// $Revision: 24 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/CharacteristicDescriptor.h"
#include "simulation/data_access/interface/CharacteristicDAI.h"
#include "core/common/Constants.h"
#include "core/utility/Utility.h"


namespace rootmap
{
#ifndef CS_USING_CHARACTERISTICDESCRIPTOR_CLASS
    // The we are using the STRUCT

    CharacteristicDescriptor::CharacteristicDescriptor()
        : id(0)
        , ScoreboardIndex(InvalidCharacteristicIndex)
        , Minimum(0)
        , Maximum(Utility::infinity())
        , Default(0)
        , Name("")
        , Units("")
        , Visible(false)
        , Edittable(false)
        , Savable(false)
        , SpecialPerBoxInfo(false)
        , m_ScoreboardStratum(ScoreboardStratum::NONE)
    {
    }


    // copy constructor
    CharacteristicDescriptor::CharacteristicDescriptor
    (const CharacteristicDescriptor& rhs,
        long int newID,
        long int newScoreboardIndex,
        const std::string& newName
    )
        : id(newID)
        , Minimum(rhs.Minimum)
        , ScoreboardIndex(newScoreboardIndex)
        , Maximum(rhs.Maximum)
        , Default(rhs.Default)
        , Name(newName)
        , Units(rhs.Units)
        , Visible(rhs.Visible)
        , Edittable(rhs.Edittable)
        , Savable(rhs.Savable)
        , SpecialPerBoxInfo(rhs.SpecialPerBoxInfo)
        , m_ScoreboardStratum(rhs.m_ScoreboardStratum)
    {
        wxString tempName = Name;
        if (tempName.Replace(" VolumeObject [none]", "") > 0)
        {
            Name = tempName;
        }
    }

    CharacteristicDescriptor::CharacteristicDescriptor(const CharacteristicDAI& data)
        : id(0)
        , Minimum(data.getMinimum())
        , ScoreboardIndex(InvalidCharacteristicIndex)
        , Maximum(data.getMaximum())
        , Default(data.getDefault())
        , Name(data.getName())
        , Units(data.getUnits())
        , Visible(data.isVisible())
        , Edittable(data.isEdittable())
        , Savable(data.isSavable())
        , SpecialPerBoxInfo(data.hasSpecialPerBoxInfo())
        , m_ScoreboardStratum(data.getStratum())
    {
        wxString tempName = Name;
        if (tempName.Replace(" VolumeObject [none]", "") > 0)
        {
            Name = tempName;
        }
    }


    CharacteristicDescriptor::~CharacteristicDescriptor()
    {
    }

    void CharacteristicDescriptor::SetName(const char* s)
    {
        wxString tempName = s;
        if (tempName.Replace(" VolumeObject [none]", "") > 0)
        {
            Name = tempName;
        }
        else
        {
            Name = s;
        }
    }

    void CharacteristicDescriptor::SetUnits(const char* s)
    {
        Units = s;
    }

    bool CharacteristicDescriptor::operator!=(const CharacteristicDescriptor& rhs) const
    {
        if ((rhs.id != id) ||
            (rhs.ScoreboardIndex != ScoreboardIndex) ||
            (rhs.Minimum != Minimum) ||
            (rhs.Maximum != Maximum) ||
            (rhs.Default != Default) ||
            (rhs.Name != Name) ||
            (rhs.Units != Units) ||
            (rhs.Visible != Visible) ||
            (rhs.Edittable != Edittable) ||
            (rhs.Savable != Savable) ||
            (rhs.SpecialPerBoxInfo != SpecialPerBoxInfo) ||
            (rhs.m_ScoreboardStratum != m_ScoreboardStratum))
        {
            return true;
        }

        return false;
    }

    bool CharacteristicDescriptor::operator==(const CharacteristicDescriptor& rhs) const
    {
        if ((rhs.id == id) &&
            (rhs.ScoreboardIndex == ScoreboardIndex) &&
            (rhs.Minimum == Minimum) &&
            (rhs.Maximum == Maximum) &&
            (rhs.Default == Default) &&
            (rhs.Name == Name) &&
            (rhs.Units == Units) &&
            (rhs.Visible == Visible) &&
            (rhs.Edittable == Edittable) &&
            (rhs.Savable == Savable) &&
            (rhs.SpecialPerBoxInfo == SpecialPerBoxInfo) &&
            (rhs.m_ScoreboardStratum == m_ScoreboardStratum))
        {
            return true;
        }

        return false;
    }

#else // using the CLASS

    // all functions are inline in the .h file
    CharacteristicDescriptor::CharacteristicDescriptor()
    {
    }


    CharacteristicDescriptor::~CharacteristicDescriptor()
    {
    }
#endif

    std::ostream& operator<<(std::ostream& ostr, const CharacteristicDescriptor& summary)
    {
        ostr << "{id:" << summary.id
            << "} {Name:" << summary.Name
            << "} {Units:" << summary.Units
            << "} {ScoreboardIndex:" << summary.ScoreboardIndex
            << "} {Minimum:" << summary.Minimum
            << "} {Maximum:" << summary.Maximum
            << "} {Default:" << summary.Default
            << "} {Visible:" << summary.Visible
            << "} {Edittable:" << summary.Edittable
            << "} {Savable:" << summary.Savable
            << "} {SpecialPerBoxInfo:" << summary.SpecialPerBoxInfo
            << "} {ScoreboardStratum:" << summary.m_ScoreboardStratum.value()
            << "}";
        return ostr;
    }

    const CharacteristicDescriptor CharacteristicDescriptor::Null;
} /* namespace rootmap */
