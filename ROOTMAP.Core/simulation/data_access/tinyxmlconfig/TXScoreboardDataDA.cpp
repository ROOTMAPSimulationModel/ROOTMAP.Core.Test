/////////////////////////////////////////////////////////////////////////////
// Name:        TXScoreboardDataDA.cpp
// Purpose:     Implementation of the TXScoreboardDataDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "TXScoreboardDataDA.h"


namespace rootmap
{
    const std::string& TXScoreboardDataDA::getProcessName() const
    {
        return m_processName;
    }

    const std::string& TXScoreboardDataDA::getCharacteristicName() const
    {
        return m_characteristicName;
    }

    SchemeContentElementList& TXScoreboardDataDA::getElementList() const
    {
        return m_elementList;
    }


    TXScoreboardDataDA::TXScoreboardDataDA
    (const std::string& process_name,
        const std::string& characteristic_name,
        SchemeContentElementList& element_list)
        : m_processName(process_name)
        , m_characteristicName(characteristic_name)
        , m_elementList(element_list)
    {
    }

    TXScoreboardDataDA::~TXScoreboardDataDA()
    {
    }
} /* namespace rootmap */

