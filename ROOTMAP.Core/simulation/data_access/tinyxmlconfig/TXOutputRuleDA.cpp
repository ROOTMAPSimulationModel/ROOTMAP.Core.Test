/////////////////////////////////////////////////////////////////////////////
// Name:        TXOutputRuleDA.cpp
// Purpose:     Implementation of the TXOutputRuleDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXOutputRuleDA.h"
#include "core/common/RmAssert.h"
#include "core/common/ExtraStuff.h"

namespace rootmap
{
    std::string TXOutputRuleDA::GetProcessName() const
    {
        return m_processName;
    }

    /**
     * @return
     */
    std::string TXOutputRuleDA::GetType() const
    {
        return m_type;
    }

    /**
     * For type=="ScoreboardData", this is the characteristic name
     * @return
     */
    std::string TXOutputRuleDA::GetName() const
    {
        return m_name;
    }

    /**
     * @return
     */
    ScoreboardStratum TXOutputRuleDA::GetStratum() const
    {
        return m_stratum;
    }

    /**
     * @return
     */
    std::string TXOutputRuleDA::GetFileName() const
    {
        return m_filename;
    }

    std::string TXOutputRuleDA::GetDirectory() const
    {
        return m_directory;
    }

    /**
     * For ScoreboardData, this is the DimensionOrder
     * @return
     */
    std::string TXOutputRuleDA::GetSpecification1() const
    {
        return m_specification1;
    }

    /**
     * @return
     */
    std::string TXOutputRuleDA::GetSpecification2() const
    {
        return m_specification2;
    }

    /**
     * @return
     */
    const OutputRuleDAI::BaseWhen& TXOutputRuleDA::GetWhen() const
    {
        RmAssert(__nullptr != m_when, "The 'When' of the OutputRule was not set before use");
        return (*m_when);
    }


    /**
     * @return
     */
    std::string TXOutputRuleDA::GetReopenStrategy() const
    {
        return m_reopenStrategy;
    }

    /**
     * Virtual destructor, like all good base class destructors should be
     */
    TXOutputRuleDA::~TXOutputRuleDA()
    {
        delete m_when;
    }

    TXOutputRuleDA::TXOutputRuleDA
    (const std::string& process_name,
        const std::string& type_name,
        const std::string& name,
        const ScoreboardStratum& stratum,
        const std::string& filename,
        const std::string& directory,
        const std::string& specification1,
        const std::string& specification2,
        const std::string& reopen_strategy)
        : m_processName(process_name)
        , m_type(type_name)
        , m_name(name)
        , m_stratum(stratum)
        , m_filename(filename)
        , m_directory(directory)
        , m_specification1(specification1)
        , m_specification2(specification2)
        , m_reopenStrategy(reopen_strategy)
        , m_when(__nullptr)
        //, m_intervalWhen(0,0,0)
        //, m_regularWhen(0,0,0,0,0,0,0)
    {
    }

    void TXOutputRuleDA::SetIntervalWhen
    (int count,
        ProcessTime_t interval,
        ProcessTime_t initialTime)
    {
        RmDelete(m_when);

        IntervalWhen* int_when = new IntervalWhen;
        int_when->m_count = count;
        int_when->m_interval = interval;
        int_when->m_initialTime = initialTime;

        m_when = int_when;
    }

    void TXOutputRuleDA::SetFixedPerRunWhen
    (int count)
    {
        RmDelete(m_when);

        FixedPerRunWhen* fwhen = new FixedPerRunWhen;
        fwhen->m_count = count;

        m_when = fwhen;
    }

    void TXOutputRuleDA::SetRegularWhen
    (int count,
        ProcessTime_t year_,
        ProcessTime_t month_,
        ProcessTime_t day_,
        ProcessTime_t hour_,
        ProcessTime_t minute_,
        ProcessTime_t second_,
        OutputRuleDAI::RegularWhen::ERegularity regularity)
    {
        RmDelete(m_when);

        RegularWhen* reg_when = new RegularWhen;

        reg_when->m_count = count;
        reg_when->m_year = year_;
        reg_when->m_month = month_;
        reg_when->m_day = day_;
        reg_when->m_hour = hour_;
        reg_when->m_minute = minute_;
        reg_when->m_second = second_;
        reg_when->m_regularity = regularity;

        m_when = reg_when;
    }
} /* namespace rootmap */


