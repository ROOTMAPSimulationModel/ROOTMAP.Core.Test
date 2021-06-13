#ifndef DataOutputCoordinator_H
#define DataOutputCoordinator_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DataOutputCoordinator.h
// Purpose:     Declaration of the DataOutputCoordinator class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/log/Logger.h"

namespace rootmap
{
    class OutputRuleDAI;
    class SimulationEngine;
    class DataAccessManager;

    /**
     * This class is responsible for coordinating the export of data from processes
     * or other classes that request it, usually at the behest of an OutputRule.
     *
     * Output = Export = Dump
     *
     * Note that the OutputRules themselves are contained and owned by the
     * OutputFileAlarm class
     */
    class DataOutputCoordinator
    {
    public:
        DataOutputCoordinator(SimulationEngine& engine, DataAccessManager& dam);
        virtual ~DataOutputCoordinator();

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        virtual void InitialiseOutput(const OutputRuleDAI& data);

        virtual const SimulationEngine& GetSimulationEngine() const;

    private:
        RootMapLoggerDeclaration();

        ///
        /// 
        SimulationEngine& m_engine;
    }; // class DataOutputCoordinator
} /* namespace rootmap */

#endif // #ifndef DataOutputCoordinator_H
