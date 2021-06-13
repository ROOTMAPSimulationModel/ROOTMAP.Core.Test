#include "BoostPrecomp.h"

/////////////////////////////////////////////////////////////////////////////
// Name:        TXPostOfficeDA.h
// Purpose:     Declaration of the TXPostOfficeDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXPostOfficeDA.h"
#include "simulation/common/BoostPrecomp.h"

namespace rootmap
{
    bool TXPostOfficeDA::hasSeed() const
    {
        return false;
    }

    unsigned int TXPostOfficeDA::getSeed() const
    {
        return 0;
    }

    ProcessTime_t TXPostOfficeDA::getPreviousTime() const
    {
        return m_previousTime;
    }


    ProcessTime_t TXPostOfficeDA::getNow() const
    {
        return m_now;
    }


    ProcessTime_t TXPostOfficeDA::getStartTime() const
    {
        return m_startTime;
    }


    ProcessTime_t TXPostOfficeDA::getNearestEnd() const
    {
        return m_nearestEnd;
    }


    ProcessTime_t TXPostOfficeDA::getDefaultRunTime() const
    {
        return m_defaultRunTime;
    }


    TXPostOfficeDA::~TXPostOfficeDA()
    {
    }

    TXPostOfficeDA::TXPostOfficeDA
    (   ProcessTime_t previousTime,
        ProcessTime_t now,
        ProcessTime_t startTime,
        ProcessTime_t nearestEnd,
        ProcessTime_t defaultRunTime)
    : m_previousTime(previousTime)
    , m_now(now)
    , m_startTime(startTime)
    , m_nearestEnd(nearestEnd)
    , m_defaultRunTime(defaultRunTime)
    {
    }


} // namespace rootmap
