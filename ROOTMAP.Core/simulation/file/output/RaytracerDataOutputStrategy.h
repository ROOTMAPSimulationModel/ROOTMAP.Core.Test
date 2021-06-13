#ifndef RaytracerDataOutputStrategy_H
#define RaytracerDataOutputStrategy_H
/////////////////////////////////////////////////////////////////////////////
// Name:        RaytracerDataOutputStrategy.h
// Purpose:     Declaration of the RaytracerDataOutputStrategy class
// Created:     19/12/2008
// Author:      MSA
// $Date: 2008-12-19 02:07:36 +0800 (Fri, 19 Dec 2008) $
// $Revision: 1 $
// Copyright:   ©2008 University of Tasmania/University of Western Australia
/////////////////////////////////////////////////////////////////////////////
#include "simulation/file/output/OutputStrategy.h"
#include "simulation/process/raytrace/RaytracerData.h"
#include "simulation/process/common/IProcessIterationUser.h"
#include "core/log/Logger.h"
#include "wx/string.h"

namespace rootmap
{
    class Process;

    class RaytracerDataOutputStrategy
        : public OutputStrategy
        , public IProcessIterationUser
    {
    public:
        /**
         * Type definition for a set of processes. A set is used here because
         * membership queries are fast (linear time for n entries).
         *
         * Is in public space so OutputStrategy can access
         */
        typedef std::set<Process *> ProcessContainer;

        RootMapLoggerDeclaration();

        RaytracerDataOutputStrategy(Process* proc,
            SimulationEngine& engine,
            const ProcessContainer& processes,
            size_t frameCount,
            bool useDF3);
        ~RaytracerDataOutputStrategy();

        /**
         *
         */
        virtual void ArrangeOutput(ProcessActionDescriptor* action, OutputRule* output_rule);

        /**
         *
         */
        virtual void OutputToFile(DataOutputFile& file);

        /**
         *
         */
        void IterateOverProcesses();

        /**
         * UseProcess
         *
         * Called by the ProcessCoordinator for each process iteration.
         *
         * @param process - the current iteration of process to be used
         */
        virtual void UseProcess(Process* process);

        /**
         * UseProcess
         *
         * Called by the ProcessCoordinator for each process per scoreboard iteration.
         *
         * @param process - the current iteration of process to be used
         * @param scoreboard - the current iteration of scoreboard to be used
         */
        virtual void UseProcess(Process* process, const ScoreboardStratum& stratum);

    private:
        Process* m_mainProcess;

        SimulationEngine& m_engine;

        /**
         * RaytracerData instance given to the processes to render into
         */
        RaytracerData* m_raytracerData;

        /**
         *
         */
         //size_t m_frameCount;


        ProcessContainer m_processes;

        /**
         * Zero-based frame index
         */
        size_t m_frameNumber;

        bool m_initialising;
    };
} /* namespace rootmap */

#endif // #ifndef RaytracerDataOutputStrategy_H
