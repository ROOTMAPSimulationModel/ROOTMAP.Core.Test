/////////////////////////////////////////////////////////////////////////////
// Name:        DummyPostOfficeDA.cpp
// Purpose:     Definition of the DummyPostOfficeDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyPostOfficeDA.h"


namespace rootmap
{
    unsigned int DummyPostOfficeDA::getSeed() const
    {
        return 0;
    }

    ProcessTime_t DummyPostOfficeDA::getPreviousTime() const
    {
        return m_previous;
    }


    ProcessTime_t DummyPostOfficeDA::getNow() const
    {
        return m_now;
    }


    ProcessTime_t DummyPostOfficeDA::getStartTime() const
    {
        return m_start;
    }


    ProcessTime_t DummyPostOfficeDA::getNearestEnd() const
    {
        return m_nearestEnd;
    }


    ProcessTime_t DummyPostOfficeDA::getDefaultRunTime() const
    {
        return m_defaultRunTime;
    }


    DummyPostOfficeDA::DummyPostOfficeDA
    (ProcessTime_t previous,
        ProcessTime_t now,
        ProcessTime_t start,
        ProcessTime_t nearest_end,
        ProcessTime_t default_run_time
    )
        : m_previous(previous)
        , m_now(now)
        , m_start(start)
        , m_nearestEnd(nearest_end)
        , m_defaultRunTime(default_run_time)
    {
    }


    DummyPostOfficeDA::~DummyPostOfficeDA()
    {
    }
} /* namespace rootmap */
