/////////////////////////////////////////////////////////////////////////////
// Name:        OutputTimingStrategy.cpp
// Purpose:     Implementation of of various OutputTimingStrategy classes
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/Types.h"
#include "simulation/data_access/interface/OutputRuleDAI.h"
#include "simulation/file/output/OutputTimingStrategy.h"
#include "simulation/process/common/Process_Dictionary.h"

#include "core/common/RmAssert.h"

#include "wx/log.h"
#include "wx/datetime.h"

#include <limits>


namespace rootmap
{
    OutputTimingStrategy::OutputTimingStrategy(int count)
        : m_maxCount(count)
        , m_currentCount(0)
        , m_previousTime(-1)
    {
    }

    OutputTimingStrategy::~OutputTimingStrategy()
    {
    }

    int OutputTimingStrategy::GetMaxCount() const
    {
        return m_maxCount;
    }

    int OutputTimingStrategy::GetCurrentCount() const
    {
        return m_currentCount;
    }

    int OutputTimingStrategy::GetRemainingCount() const
    {
        if (m_maxCount > 0)
        {
            return (m_maxCount - m_currentCount);
        }
        else
        {
            return std::numeric_limits<int>::max();
        }
    }


    void OutputTimingStrategy::UpdateTime(ProcessTime_t t)
    {
        RmAssert(t > m_previousTime, "UpdateTime should provide later time than previously saved");
        m_previousTime = t;
    }

    ProcessTime_t OutputTimingStrategy::GetPreviousTime() const
    {
        return m_previousTime;
    }

    void OutputTimingStrategy::IncrementCount()
    {
        ++m_currentCount;
    }


    TimestampOutputTimingStrategy::TimestampOutputTimingStrategy(ProcessTime_t timestamp)
        : OutputTimingStrategy(1)
        , m_timestamp(timestamp)
    {
    }

    ProcessTime_t TimestampOutputTimingStrategy::GetNextTime(ProcessTime_t /* t */) const
    {
        return m_timestamp;
    }

    bool TimestampOutputTimingStrategy::DoesThisTime(ProcessTime_t t) const
    {
        // does this timestamp if a) timestamp matches
        return (t == m_timestamp);
    }


    IntervalOutputTimingStrategy::IntervalOutputTimingStrategy(int count, ProcessTime_t interval, ProcessTime_t initialTime)
        : OutputTimingStrategy(count)
        , m_interval(interval)
        , m_initialTime(initialTime)
    {
    }

    // MSA 10.03.11 Modified this function to use a more reliable method of counting intervals passed
    // (removed the use of integer division)
    ProcessTime_t IntervalOutputTimingStrategy::GetNextTime(ProcessTime_t t) const
    {
        if (m_interval == 0)
        {
            return (ProcessTime_t)INT_MAX;
        }

        if (m_initialTime > t)
        {
            return m_initialTime;
        }

        if (GetRemainingCount() < 1)
        {
            return t;
        }

        ProcessTime_t nextTime = m_initialTime;
        while (nextTime <= t)
        {
            nextTime += m_interval;
        }

        return nextTime;
    }

    bool IntervalOutputTimingStrategy::DoesThisTime(ProcessTime_t t) const
    {
        return m_interval != 0
            && ((0 == ((t - m_initialTime) % m_interval)) && (GetRemainingCount() > 0));
    }


    RootMapLoggerDefinition(RegularOutputTimingStrategy);

    RegularOutputTimingStrategy::RegularOutputTimingStrategy(int count, ProcessTime_t regularTime, RegularOutputInterval regularity)
        : OutputTimingStrategy(count)
        , m_regularTime(regularTime)
        , m_regularity(regularity)
    {
        RootMapLoggerInitialisation("rootmap.RegularOutputTimingStrategy");
    }

    ProcessTime_t RegularOutputTimingStrategy::GetNextTime(ProcessTime_t t) const
    {
        if (GetRemainingCount() < 1)
        {
            return t;
        }

        wxDateTime t_wx(t);
        wxDateTime regular_wx(m_regularTime);

        switch (m_regularity)
        {
        case year:
            t_wx.SetYear(t_wx.GetYear() + 1);
            t_wx.SetMonth(regular_wx.GetMonth());
            t_wx.SetDay(regular_wx.GetDay());
            t_wx.SetHour(regular_wx.GetHour());
            t_wx.SetMinute(regular_wx.GetMinute());
            t_wx.SetSecond(regular_wx.GetSecond());
            break;

        case month:
        {
            wxDateSpan month_span(0, 1, 0, 0);
            t_wx.Add(month_span);
            t_wx.SetDay(regular_wx.GetDay());
            t_wx.SetHour(regular_wx.GetHour());
            t_wx.SetMinute(regular_wx.GetMinute());
            t_wx.SetSecond(regular_wx.GetSecond());
        }
        break;

        case day:
            t_wx.Add(wxTimeSpan::Day());
            t_wx.SetHour(regular_wx.GetHour());
            t_wx.SetMinute(regular_wx.GetMinute());
            t_wx.SetSecond(regular_wx.GetSecond());
            break;

        case hour:
            t_wx.Add(wxTimeSpan::Hour());
            t_wx.SetMinute(regular_wx.GetMinute());
            t_wx.SetSecond(regular_wx.GetSecond());
            break;

        case minute:
            t_wx.Add(wxTimeSpan::Minute());
            t_wx.SetSecond(regular_wx.GetSecond());
            break;

        case second:
        default:
            LOG_ERROR << "GetNextTime(" << t << ") not yet implemented for second regularity";
            break;
        }

        ProcessTime_t t_next = t_wx.GetTicks();
        LOG_INFO << "GetNextTime(" << t << ") returns " << t_next;
        return t_next;
    }

    bool RegularOutputTimingStrategy::DoesThisTime(ProcessTime_t t) const
    {
        // return if we have already exceeded count
        if (GetRemainingCount() < 1)
        {
            return false;
        }

        wxDateTime t_wx(t);
        wxDateTime regular_wx(m_regularTime);

        switch (m_regularity)
        {
        case year:
            return ((regular_wx.GetMonth() == t_wx.GetMonth())
                && (regular_wx.GetDay() == t_wx.GetDay())
                && (t_wx.IsSameTime(regular_wx)));

        case month:
            return ((regular_wx.GetDay() == t_wx.GetDay())
                && (t_wx.IsSameTime(regular_wx)));

        case day:
            return (t_wx.IsSameTime(regular_wx));

        case hour:
            return ((regular_wx.GetMinute() == t_wx.GetMinute())
                && (regular_wx.GetSecond() == t_wx.GetSecond())
                );

        case minute:
            return (regular_wx.GetSecond() == t_wx.GetSecond());

        case second:
        default:
            LOG_ERROR << "DoesThisTime(" << t << ") not yet implemented for second regularity";
            return false;
        }
    }


    RootMapLoggerDefinition(OutputTimingStrategyFactory);

    OutputTimingStrategy* OutputTimingStrategyFactory::createOutputTimingStrategy(const OutputRuleDAI& data, const SimulationEngine& /* engine */)
    {
        RootMapLoggerInitialisation("rootmap.OutputTimingStrategyFactory");
        const OutputRuleDAI::BaseWhen& when = data.GetWhen();

        if (when.GetTypeName() == "Interval")
        {
            try
            {
                const OutputRuleDAI::IntervalWhen& iwhen = dynamic_cast<const OutputRuleDAI::IntervalWhen &>(when);

                OutputTimingStrategy* outimstrategy = new IntervalOutputTimingStrategy(iwhen.m_count, iwhen.m_interval, iwhen.m_initialTime);
                LOG_INFO << "Created IntervalOutputTimingStrategy with count:" << iwhen.m_count << ", interval:" << iwhen.m_interval << ", initialtime:" << iwhen.m_initialTime;
                return outimstrategy;
            }
            catch (const std::bad_cast&)
            {
                LOG_ERROR << LOG_LINE << "Failed to cast BaseWhen instance that claimed to be 'Interval' to IntervalWhen";
                RmAssert(false, "Failed to cast BaseWhen instance that claimed to be 'Interval' to IntervalWhen");
            }
        }
        // if the <when> has <attime> and <regularity> set
        else if (when.GetTypeName() == "Regular")
        {
            try
            {
                const OutputRuleDAI::RegularWhen& rwhen = dynamic_cast<const OutputRuleDAI::RegularWhen &>(when);

                ProcessTime_t t = PROCESS_YEARS(rwhen.m_year)
                    + PROCESS_MONTHS(rwhen.m_month)
                    + PROCESS_DAYS(rwhen.m_day)
                    + PROCESS_HOURS(rwhen.m_hour)
                    + PROCESS_MINUTES(rwhen.m_minute)
                    + rwhen.m_second;
                if (1 == rwhen.m_count)
                {
                    OutputTimingStrategy* strategy = new TimestampOutputTimingStrategy(t);
                    LOG_INFO << "Created TimestampOutputTimingStrategy at time:" << t;
                    return strategy;
                }
                else
                {
                    RegularOutputTimingStrategy::RegularOutputInterval regularity = RegularOutputTimingStrategy::unset;
                    switch (rwhen.m_regularity)
                    {
                    case OutputRuleDAI::RegularWhen::r_year: regularity = RegularOutputTimingStrategy::year;
                        break;
                    case OutputRuleDAI::RegularWhen::r_month: regularity = RegularOutputTimingStrategy::month;
                        break;
                    case OutputRuleDAI::RegularWhen::r_day: regularity = RegularOutputTimingStrategy::day;
                        break;
                    case OutputRuleDAI::RegularWhen::r_hour: regularity = RegularOutputTimingStrategy::hour;
                        break;
                    case OutputRuleDAI::RegularWhen::r_minute: regularity = RegularOutputTimingStrategy::minute;
                        break;
                    case OutputRuleDAI::RegularWhen::r_second: regularity = RegularOutputTimingStrategy::second;
                        break;
                    case OutputRuleDAI::RegularWhen::unset: default: regularity = RegularOutputTimingStrategy::unset;
                    }
                    OutputTimingStrategy* strategy = new RegularOutputTimingStrategy(rwhen.m_count, t, regularity);
                    LOG_INFO << "Created RegularOutputTimingStrategy for count:" << rwhen.m_count << ", time:" << t << ", regularity:" << static_cast<int>(regularity);
                    return strategy;
                }
            }
            catch (const std::bad_cast&)
            {
                LOG_ERROR << LOG_LINE << "Failed to cast BaseWhen instance that claimed to be 'Regular' to RegularWhen";
                RmAssert(false, "Failed to cast BaseWhen instance that claimed to be 'Regular' to RegularWhen");
            }
        }

        RmAssert(false, "OutputTimingStrategyFactory::createOutputTimingStrategy not yet correctly implemented");
        return __nullptr;
    }
} /* namespace rootmap */
