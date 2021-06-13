/////////////////////////////////////////////////////////////////////////////
// Name:        TXProcessDA.cpp
// Purpose:     Definition of the TXProcessDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXProcessDA.h"
#include "simulation/data_access/tinyxmlconfig/TXCharacteristicDA.h"

#include "core/common/RmAssert.h"


namespace rootmap
{
    ProcessIdentifier TXProcessDA::getIdentifier() const
    {
        return m_identifier;
    }


    std::string TXProcessDA::getName() const
    {
        return m_name;
    }


    ScoreboardStratum TXProcessDA::getStratum() const
    {
        return m_stratum;
    }


    ProcessActivity TXProcessDA::getActivity() const
    {
        return m_activity;
    }

    const CharacteristicDAICollection& TXProcessDA::getCharacteristics() const
    {
        return m_characteristics;
    }

    void TXProcessDA::addCharacteristicDA(TXCharacteristicDA* characteristicDA)
    {
        RmAssert(__nullptr != characteristicDA, "NULL characteristic data given to addCharacteristicDA()");

        m_characteristics.push_back(characteristicDA);
    }

    bool TXProcessDA::doesOverride() const
    {
        return m_doesOverride;
    }

    bool TXProcessDA::isValid() const
    {
        // the values in this class are all set during construction so those are safe
        // the values in the CharacteristicDA's are all set during construction,
        // those that cannot be are not added to this ProcessDA. Therefore this
        // data should be valid

        return true;
    }


    TXProcessDA::~TXProcessDA()
    {
        for (CharacteristicDAICollection::iterator iter(m_characteristics.begin());
            iter != m_characteristics.end(); ++iter)
        {
            delete (*iter);
        }
    }


    TXProcessDA::TXProcessDA
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
