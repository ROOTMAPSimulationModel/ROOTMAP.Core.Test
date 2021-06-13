#ifndef TXOutputRuleDA_H
#define TXOutputRuleDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXOutputRuleDA.h
// Purpose:     Declaration of the TXOutputRuleDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/Types.h"
#include "simulation/data_access/interface/OutputRuleDAI.h"

#include "wx/string.h"

#include <vector>

namespace rootmap
{
    class Process;

    class TXOutputRuleDA : public OutputRuleDAI
    {
    public:
        /**
         * @return
         */
        virtual std::string GetProcessName() const;

        /**
         * @return
         */
        virtual std::string GetType() const;

        /**
         * For type=="ScoreboardData", this is the characteristic name
         * @return
         */
        virtual std::string GetName() const;

        /**
         * @return
         */
        virtual ScoreboardStratum GetStratum() const;

        /**
         * @return
         */
        virtual std::string GetFileName() const;

        /**
         * Accessor for the output directory absolute path
         */
        virtual std::string GetDirectory() const;


        /**
         * For ScoreboardData, this is the DimensionOrder
         * @return
         */
        virtual std::string GetSpecification1() const;

        /**
         * @return
         */
        virtual std::string GetSpecification2() const;

        /**
         * As opposed to IsRegularWhen (the logical negation of this function)
         * @return
         */
        virtual const BaseWhen& GetWhen() const;

        /**
         * @return
         */
        virtual std::string GetReopenStrategy() const;


        /**
         * Virtual destructor, like all good base class destructors should be
         */
        virtual ~TXOutputRuleDA();

        TXOutputRuleDA(const std::string& process_name,
            const std::string& type_name,
            const std::string& name,
            const ScoreboardStratum& stratum,
            const std::string& filename,
            const std::string& directory,
            const std::string& specification1,
            const std::string& specification2,
            const std::string& reopen_strategy);

        void SetIntervalWhen(int count,
            ProcessTime_t interval,
            ProcessTime_t initialTime);

        void SetRegularWhen(int count,
            ProcessTime_t year_,
            ProcessTime_t month_,
            ProcessTime_t day_,
            ProcessTime_t hour_,
            ProcessTime_t minute_,
            ProcessTime_t second_,
            OutputRuleDAI::RegularWhen::ERegularity regularity);

        void SetFixedPerRunWhen(int count);


    private:
        std::string m_processName;
        std::string m_type;
        std::string m_name;
        ScoreboardStratum m_stratum;
        std::string m_filename;
        std::string m_directory;
        std::string m_specification1;
        std::string m_specification2;
        std::string m_reopenStrategy;

        BaseWhen* m_when;
    }; // class TXOutputRuleDA
} /* namespace rootmap */

#endif // #ifndef TXOutputRuleDA_H
