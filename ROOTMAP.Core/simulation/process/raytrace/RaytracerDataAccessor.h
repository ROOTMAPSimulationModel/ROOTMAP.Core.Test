#ifndef RaytracerDataAccessor_H
#define RaytracerDataAccessor_H
/////////////////////////////////////////////////////////////////////////////
// Name:        RaytracerDataAccessor.h
// Purpose:     Declaration of the RaytracerDataAccessor class
// Created:     09/01/2009
// Author:      MSA
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2009 University of Tasmania/University of Western Australia
//
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/IProcessIterationUser.h"
#include "simulation/process/raytrace/RaytracerData.h"
#include "core/log/Logger.h"
#include "simulation/process/common/ProcessCoordinator.h"


namespace rootmap
{
    class Process;
    class SimulationEngine;

    class RaytracerDataAccessor
        : public IProcessIterationUser
    {
    public:
        RaytracerDataAccessor(SimulationEngine& engine);
        virtual ~RaytracerDataAccessor();

        /**
         * If this method is never called, all Processes are used.
         */
        void addAllowedName(const std::string& name);

        /**
         * If this method is never called, all Processes are used.
         */
        void addProcess(Process* p);


        void DoCache(RaytracerData* rdp);

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

        RootMapLoggerDeclaration();

        ///
        ProcessCoordinator& m_processCoord;

        RaytracerData* m_raytracerDataPointer;

        //issue109msg118
        //std::vector<std::string> m_allowedNames;

        /**
         * Type definition for a set of processes. A set is used here because
         * membership queries are fast (linear time for n entries).
         */
        typedef std::set<Process *> ProcessContainer;

        ProcessContainer m_processes;

        //issue109msg118
        //bool m_setting;
        //bool m_caching;
    }; // class RaytracerDataAccessor
} /* namespace rootmap */
#endif // #ifndef RaytracerDataAccessor_H
