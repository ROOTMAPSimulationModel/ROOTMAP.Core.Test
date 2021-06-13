#ifndef OutputTimingStrategy_H
#define OutputTimingStrategy_H
/////////////////////////////////////////////////////////////////////////////
// Name:        OutputTimingStrategy.h
// Purpose:     Contains declarations of various OutputTimingStrategy classes
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
// 
// Declarations for strategies used by OutputRule are in here. These consist
// of
//
// OutputTimingStrategy - the base class, used by the OutputRule
/////////////////////////////////////////////////////////////////////////////
#include "core/log/Logger.h"

namespace rootmap
{
    class OutputRuleDAI;
    class SimulationEngine;

    /**
     * Output timing strategy base class
     */
    class OutputTimingStrategy
    {
    public:
        /**
         *
         */
        OutputTimingStrategy(int count);

        /**
         *
         */
        virtual ~OutputTimingStrategy();

        /**
         *
         */
        int GetMaxCount() const;

        /**
         *
         */
        int GetCurrentCount() const;

        /**
         *
         */
        int GetRemainingCount() const;

        /**
         *
         */
        virtual ProcessTime_t GetNextTime(ProcessTime_t t) const = 0;

        /**
         *
         */
        virtual bool DoesThisTime(ProcessTime_t t) const = 0;

        /**
         *
         */
        void UpdateTime(ProcessTime_t t);

        /**
         *
         */
        void IncrementCount();

    protected:
        /**
         *
         */
        ProcessTime_t GetPreviousTime() const;

    private:

        ///
        /// 
        int m_maxCount;

        ///
        /// 
        int m_currentCount;

        ///
        /// 
        ProcessTime_t m_previousTime;
    }; // class OutputTimingStrategy


    /**
     * Intervals
     */
    class IntervalOutputTimingStrategy : public OutputTimingStrategy
    {
    public:
        /**
         *
         */
        IntervalOutputTimingStrategy(int count, ProcessTime_t interval, ProcessTime_t initialTime);

        /**
         *
         */
        ProcessTime_t GetNextTime(ProcessTime_t t) const;

        /**
         *
         */
        bool DoesThisTime(ProcessTime_t t) const;

    private:
        ///
        ///
        ProcessTime_t m_interval;

        ///
        ///
        ProcessTime_t m_initialTime;
    };


    /**
     * Single Timestamp
     */
    class TimestampOutputTimingStrategy : public OutputTimingStrategy
    {
    public:
        /**
         *
         */
        TimestampOutputTimingStrategy(ProcessTime_t timestamp);

        /**
         *
         */
        ProcessTime_t GetNextTime(ProcessTime_t t) const;

        /**
         *
         */
        bool DoesThisTime(ProcessTime_t t) const;

    private:
        ///
        ///
        ProcessTime_t m_timestamp;
    }; // class TimestampOutputTimingStrategy


    /**
     * Regular Timestamp
     */
    class RegularOutputTimingStrategy : public OutputTimingStrategy
    {
    public:

        enum RegularOutputInterval
        {
            second = 1,
            minute = 2,
            hour = 3,
            day = 4,
            month = 5,
            year = 6,
            unset = 0xFF
        };

        /**
         *
         */
        RegularOutputTimingStrategy(int count, ProcessTime_t regularTime, RegularOutputInterval regularity);

        /**
         *
         */
        ProcessTime_t GetNextTime(ProcessTime_t t) const;

        /**
         *
         */
        bool DoesThisTime(ProcessTime_t t) const;

    private:
        RootMapLoggerDeclaration();
        ///
        ///
        ProcessTime_t m_regularTime;

        ///
        ///
        RegularOutputInterval m_regularity;
    };


    class OutputTimingStrategyFactory
    {
    public:
        static OutputTimingStrategy* createOutputTimingStrategy(const OutputRuleDAI& data, const SimulationEngine& engine);

        RootMapLoggerDeclaration();
    };
} /* namespace rootmap */

#endif // #ifndef OutputTimingStrategy_H
