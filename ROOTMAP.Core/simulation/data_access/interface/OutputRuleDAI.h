#ifndef OutputRuleDAI_H
#define OutputRuleDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        OutputRuleDAI.h
// Purpose:     Declaration of the OutputRuleDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/scoreboard/ScoreboardStratum.h"
#include "simulation/process/common/Process_Dictionary.h"
#include "simulation/common/Types.h"

#include <vector>

namespace rootmap
{
    class Process;

    class OutputRuleDAI
    {
    public:
        struct BaseWhen
        {
            int m_count;

            BaseWhen(int count, const char* name) : m_count(count), m_name(name)
            {
            }

            virtual ~BaseWhen()
            {
            }

            const std::string& GetTypeName() const;
        private:
            std::string m_name;
        };

        struct IntervalWhen : public BaseWhen
        {
            ProcessTime_t m_interval;
            ProcessTime_t m_initialTime;

            IntervalWhen()
                : BaseWhen(0, "Interval")
                , m_interval(0), m_initialTime(0)
            {
            }
        };

        struct RegularWhen : public BaseWhen
        {
            ProcessTime_t m_year;
            ProcessTime_t m_month;
            ProcessTime_t m_day;
            ProcessTime_t m_hour;
            ProcessTime_t m_minute;
            ProcessTime_t m_second;

            ///
            /// all of the enumerated values are prefixed with r_ to minimise naming conflicts
            enum ERegularity
            {
                unset,
                r_year,
                r_month,
                r_day,
                r_hour,
                r_minute,
                r_second
            };

            ERegularity m_regularity;

            RegularWhen()
                : BaseWhen(0, "Regular")
                , m_year(-1), m_month(-1), m_day(-1)
                , m_hour(-1), m_minute(-1), m_second(-1)
                , m_regularity(unset)
            {
            }

            ProcessTime_t GetInitialTime() const
            {
                return PROCESS_YEARS(m_year) + PROCESS_MONTHS(m_month) +
                    PROCESS_DAYS(m_day) + PROCESS_HOURS(m_hour) + PROCESS_MINUTES(m_minute) + m_second;
            }
        };

        struct FixedPerRunWhen : public BaseWhen
        {
            FixedPerRunWhen()
                : BaseWhen(0, "FixedPerRun")
            {
            }
        };

        /**
         * @return
         */
        virtual std::string GetProcessName() const = 0;

        /**
         * @return
         */
        virtual std::string GetType() const = 0;

        /**
         * For type=="ScoreboardData", this is the characteristic name
         * @return
         */
        virtual std::string GetName() const = 0;

        /**
         * @return
         */
        virtual ScoreboardStratum GetStratum() const = 0;

        /**
         * @return
         */
        virtual std::string GetFileName() const = 0;

        /**
         * @return
         */
        virtual std::string GetDirectory() const = 0;

        /**
         * For ScoreboardData, this is the DimensionOrder
         * For Raytracer, this is the comma-separated list of process names
         * @return
         */
        virtual std::string GetSpecification1() const = 0;

        /**
         * @return
         */
        virtual std::string GetSpecification2() const = 0;

        /**
         * @return
         */
        virtual const BaseWhen& GetWhen() const = 0;

        /**
         * @return
         */
        virtual std::string GetReopenStrategy() const = 0;

        /**
         * Virtual destructor, like all good base class destructors should be
         */
        virtual ~OutputRuleDAI()
        {
        }

    protected:
        OutputRuleDAI()
        {
        }
    }; // class OutputRuleDAI

    inline const std::string& OutputRuleDAI::BaseWhen::GetTypeName() const
    {
        return m_name;
    }
} /* namespace rootmap */

#endif // #ifndef OutputRuleDAI_H
