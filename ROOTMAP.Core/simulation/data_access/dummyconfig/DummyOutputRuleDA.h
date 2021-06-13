#ifndef DummyOutputRuleDA_H
#define DummyOutputRuleDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyOutputRuleDA.h
// Purpose:     Declaration of the DummyOutputRuleDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/OutputRuleDAI.h"

namespace rootmap
{
    class DummyOutputRuleDA : public OutputRuleDAI
    {
    public:

        /**
         * The owner of the data to be export, not of the output association.
         */
        virtual std::string GetProcess() const;

        /**
         * Type determines the Output algorithm - is usually ScoreboardData
         */
        virtual std::string GetType() const;


        /**
         * The name is meaningful for the owner/type combination. This tells the
         * data owner what to output
         */
        virtual std::string GetName() const;

        /**
         * Accessor for the ScoreboardStratum to output
         */
        virtual ScoreboardStratum GetStratum() const;

        /**
         * Accessor for the filename template, replete with % format specifiers
         */
        virtual std::string GetFileName() const;

        /**
         * Accessor for the output directory absolute path
         */
        virtual std::string GetDirectory() const;

        /**
         * Mostly only meaningful to type==ScoreboardData
         */
        virtual std::string GetSpecification1() const;

        /**
         * @return
         */
        virtual std::string GetSpecification2() const;

        /**
         *
         * @return
         */
        virtual const BaseWhen& GetWhen() const;

        /**
         * @return
         */
        virtual std::string GetReopenStrategy() const;

    public:
        virtual ~DummyOutputRuleDA();
        DummyOutputRuleDA();

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */

        OutputRuleDAI::IntervalWhen m_when;
    }; // class DummyOutputRuleDA
} /* namespace rootmap */

#endif // #ifndef DummyOutputRuleDA_H
