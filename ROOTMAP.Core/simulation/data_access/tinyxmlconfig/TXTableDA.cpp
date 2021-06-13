/////////////////////////////////////////////////////////////////////////////
// Name:        TXTableDA.cpp
// Purpose:     Implementation of the TXTableDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXTableDA.h"

namespace rootmap
{
    TXTableDA::TXTableDA
    (const char* processName,
        const char* characteristicName,
        ViewDirection viewDir,
        long layer,
        const ScoreboardStratum& stratum)
        : m_processName(processName)
        , m_characteristicName(characteristicName)
        , m_viewDirection(viewDir)
        , m_layer(layer)
        , m_stratum(stratum)
    {
    }

    TXTableDA::~TXTableDA()
    {
    }

    std::string TXTableDA::getProcessName() const
    {
        return m_processName;
    }

    std::string TXTableDA::getCharacteristicName() const
    {
        return m_characteristicName;
    }

    ViewDirection TXTableDA::getViewDirection() const
    {
        return m_viewDirection;
    }

    long TXTableDA::getLayer() const
    {
        return m_layer;
    }

    ScoreboardStratum TXTableDA::getScoreboardStratum() const
    {
        return m_stratum;
    }
} /* namespace rootmap */
