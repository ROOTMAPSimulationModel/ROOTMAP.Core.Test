/////////////////////////////////////////////////////////////////////////////
// Name:        RaytracerDataAccessor.cpp
// Purpose:     Declaration of the RaytracerDataAccessor class
// Created:     09/01/2009
// Author:      MSA
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2009 University of Tasmania/University of Western Australia
//
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/raytrace/RaytracerDataAccessor.h"
#include "simulation/common/SimulationEngine.h"
#include "simulation/process/common/Process.h"


namespace rootmap
{
    RootMapLoggerDefinition(RaytracerDataAccessor);

    RaytracerDataAccessor::RaytracerDataAccessor(SimulationEngine& engine)
        : m_processCoord(engine.GetProcessCoordinator())
        , m_raytracerDataPointer(__nullptr)
    {
        RootMapLoggerInitialisation("rootmap.RaytracerDataAccessor");
        LOG_DEBUG << "RaytracerDataAccessor constructor";
    }

    RaytracerDataAccessor::~RaytracerDataAccessor()
    {
    }

    void RaytracerDataAccessor::addAllowedName(const std::string& name)
    {
        // instead of allowing names, add processes directly. Then we can
        // iterate directly over the processes we know about, instead of having
        // to assign a RaytracerData object to them

        Process* p = m_processCoord.FindProcessByProcessName(name);
        if (__nullptr != p)
        {
            addProcess(p);
        }

        LOG_DEBUG << "Added allowed name {Name:" << name << "}";
    }

    void RaytracerDataAccessor::addProcess(Process* process)
    {
        m_processes.insert(process);
        LOG_DEBUG << "Added process {Name:" << process->GetProcessName() << "}";
    }

    void RaytracerDataAccessor::DoCache(RaytracerData* rdp)
    {
        m_raytracerDataPointer = rdp;

        // If no processes are specified, use all of them
        if (m_processes.empty())
        {
            m_processCoord.IterateOverProcesses(this);
        }
        // otherwise, just iterate over those that are specified
        else
        {
            for (ProcessContainer::iterator iter(m_processes.begin());
                iter != m_processes.end();
                ++iter)
            {
                UseProcess(*iter);
            }
        }
    }


    void RaytracerDataAccessor::UseProcess(Process* process)
    {
        LOG_DEBUG << "Caching process {Name:" << process->GetProcessName() << "}";

        if (process->DoesRaytracerOutput())
        {
            process->DoRaytracerOutput(m_raytracerDataPointer);
        }
    }

    void RaytracerDataAccessor::UseProcess(Process* /* process */, const ScoreboardStratum& /* stratum */)
    {
        // not required to be implemented
    }
} /* namespace rootmap */

