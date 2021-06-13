/////////////////////////////////////////////////////////////////////////////
// Name:        RaytracerDataOutputStrategy.cpp
// Purpose:     Implementation of the RaytracerDataOutputStrategy class
// Created:     19/12/2008
// Author:      MSA
// $Date: 2008-12-19 02:07:36 +0800 (Fri, 19 Dec 2008) $
// $Revision: 1 $
// Copyright:   ©2008 University of Tasmania/University of Western Australia
/////////////////////////////////////////////////////////////////////////////
#include "simulation/file/output/RaytracerDataOutputStrategy.h"
#include "simulation/file/output/DataOutputFile.h"
#include "simulation/file/output/OutputRule.h"
#include "simulation/process/common/Process.h"
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "simulation/process/raytrace/RaytracerDataAccessor.h"
#include "simulation/common/SimulationEngine.h"
#include "core/common/RmAssert.h"

#include "wx/filename.h"

#define ROOTMAP_RAYTRACER_OUTPUT_STRING_OPTIMISATION 1

namespace rootmap
{
    RootMapLoggerDefinition(RaytracerDataOutputStrategy);

    RaytracerDataOutputStrategy::RaytracerDataOutputStrategy
    (Process* proc,
        SimulationEngine& engine,
        const ProcessContainer& processes,
        size_t frameCount,
        bool useDF3)
        : m_mainProcess(proc)
        , m_engine(engine)
        , m_raytracerData(__nullptr)
        , m_processes(processes)
        , m_frameNumber(0)
        , m_initialising(true)
    {
        RootMapLoggerInitialisation("rootmap.RaytracerDataOutputStrategy");

        // NULL Process pointer not allowed, is prevented in OutputStrategy::createStrategy
        RmAssert(__nullptr != m_mainProcess, "NULL Process provided for Raytracer data output strategy");
        RmAssert(m_mainProcess->DoesRaytracerOutput(), "Raytracer data output strategy requires a Process that can perform raytracer output");

        m_raytracerData = new RaytracerData(proc->GetOrigin(), useDF3);
        m_raytracerData->SetFrameCount(frameCount);

        // Tell all interested parties that they need to generate raytracer data.
        // The first call of this method just switches raytracer data output on
        // for all target processes.
        IterateOverProcesses();
    }

    /**
     *  Note that the output strategy is considered to be the owner of the RaytracerData pointer.
     *  Users of this pointer should neither delete it nor set it to null upon destruction -
     *  this destructor handles all that.
     */
    RaytracerDataOutputStrategy::~RaytracerDataOutputStrategy()
    {
        delete m_raytracerData;
    }

    void RaytracerDataOutputStrategy::ArrangeOutput(ProcessActionDescriptor* action, OutputRule* output_rule)
    {
        Use_PostOffice;

        // Ready the RaytracerData
        m_raytracerData->StartNextFrame();

        // Tell all interested parties to take a snapshot for raytracing
        // This will be the second or subsequent call of this method,
        // which will actually do output.
        IterateOverProcesses();

        RmAssert(0 != m_mainProcess, "Must have process to send SpecialOutputData message to");
        postoffice->sendMessage(m_mainProcess, m_mainProcess, kSpecialOutputDataMessage, output_rule);
    }

    void RaytracerDataOutputStrategy::OutputToFile(DataOutputFile& file)
    {
        LOG_DEBUG << "OutputToFile Start {File:" << file.GetFilename() << "}";
        // MSA 11.05.14 Performance tweaking
#ifdef ROOTMAP_RAYTRACER_OUTPUT_STRING_OPTIMISATION
        bool doContinue = true;
        bool placeholderAdded = false;
        do
        {
            const wxString& str = m_raytracerData->GetNextFrameStringSegment(m_frameNumber, file.GetFilename(), placeholderAdded);
            doContinue = !placeholderAdded && !str.empty();
            if (doContinue) file.WriteString(str);
        } while (doContinue);
#else
        const wxString & str = m_raytracerData->GetFrameString(m_frameNumber, file.GetFilename());
        file.WriteString(str);
#endif
        ++m_frameNumber;
        LOG_DEBUG << "OutputToFile End";
    }

    void RaytracerDataOutputStrategy::IterateOverProcesses()
    {
        // If no processes are specified, use all of them
        if (m_processes.empty())
        {
            m_engine.GetProcessCoordinator().IterateOverProcesses(this);
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
        m_initialising = false;
    }


    void RaytracerDataOutputStrategy::UseProcess(Process* process)
    {
        if (process->DoesRaytracerOutput())
        {
            if (m_initialising)
            {
                process->DoActivateRaytracerOutput();
            }
            else
            {
                LOG_DEBUG << "DoRaytracerOutput Start {ProcessName:" << process->GetProcessName() << "}";
                process->DoRaytracerOutput(m_raytracerData);
                LOG_DEBUG << "DoRaytracerOutput End {ProcessName:" << process->GetProcessName() << "}";
            }
        }
    }

    void RaytracerDataOutputStrategy::UseProcess(Process* /* process */, const ScoreboardStratum& /* stratum */)
    {
        // not required to be implemented
    }
} /* namespace rootmap */
