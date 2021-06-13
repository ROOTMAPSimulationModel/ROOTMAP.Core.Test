#ifndef OutputStrategy_H
#define OutputStrategy_H
/////////////////////////////////////////////////////////////////////////////
// Name:        OutputStrategy.h
// Purpose:     Contains declarations of various OutputStrategy classes
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
// 
// Declarations for strategies used by OutputRule are in here. These consist
// of
//
// OutputStrategy - the base class, used by the OutputRule
/////////////////////////////////////////////////////////////////////////////
#include "core/log/Logger.h"

namespace rootmap
{
    class OutputRule;
    class OutputRuleDAI;
    class DataOutputFile;
    class SimulationEngine;
    class ProcessActionDescriptor;

    /**
     * Output timing strategy base class
     *
     * Provides behaviour that actually performs the source->destination output format
     */
    class OutputStrategy
    {
    public:
        /**
         *
         */
        OutputStrategy()
        {
        }

        /**
         *
         */
        virtual ~OutputStrategy()
        {
        }

        /**
         *
         */
        virtual void OutputToFile(DataOutputFile& file) = 0;

        /**
         * This is called by OutputFileAlarm::InvokeOutputRules, when it is
         * already decided that this output is to export during this timestamp.
         *
         * The default behaviour of this function should be to send a kSpecialOutputDataMessage
         * to the destination object (usually a Process).  Using that message type
         * will ensure that the output is processed after everything else within
         * the given timestamp, including all registrations and cough-ups which
         * might affect the values that the destination might want to output.
         */
        virtual void ArrangeOutput(ProcessActionDescriptor* action, OutputRule* output_rule) = 0;

        // Static local utility method for determining number of output events required from a "repeat ad infinitum" setting
        static const size_t CalculateNumberOfOutputEvents(const OutputRuleDAI& data, SimulationEngine& engine);
        ///
        /// 
    }; // class OutputStrategy


    /**
     * Intervals
     */


    class OutputStrategyFactory
    {
    public:
        /**
         * Using the OutputRuleDAI and the SimulationEngine, creates an OutputStrategy
         */
        static OutputStrategy* createOutputStrategy(const OutputRuleDAI& data,
            SimulationEngine& engine);

        RootMapLoggerDeclaration();
    };
} /* namespace rootmap */

#endif // #ifndef OutputStrategy_H
