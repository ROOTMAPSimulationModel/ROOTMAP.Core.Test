/////////////////////////////////////////////////////////////////////////////
// Name:        OutputFileAlarm.cpp
// Purpose:     Implementation of the OutputFileAlarm class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/file/output/OutputFileAlarm.h"
#include "simulation/file/output/OutputRule.h"
#include "simulation/file/output/DataOutputFile.h"

#include "core/utility/Utility.h"

#include <limits>


namespace rootmap
{
    IMPLEMENT_DYNAMIC_CLASS(OutputFileAlarm, Process)


        OutputFileAlarm::OutputFileAlarm()
    {
    }

    OutputFileAlarm::~OutputFileAlarm()
    {
        for (OutputRuleCollection::iterator it(m_outputRules.begin());
            it != m_outputRules.end(); ++it)
        {
            delete (*it);
        }
    }


    long int OutputFileAlarm::Initialise(ProcessActionDescriptor* action)
    {
        SendNextOutputTime(action);

        return (0);
    }


    long int OutputFileAlarm::DoNormalWakeUp(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        return_value = InvokeOutputRules(action);

        return_value |= Process::DoNormalWakeUp(action);

        return (return_value);
    }


    long int OutputFileAlarm::DoExternalWakeUp(ProcessActionDescriptor* action)
    {
        /* the protocol between this function and DataOutputCoordinator::InitialiseOutput
        is that it sends "this" process as the source of a kExternalWakeUpMessage,
        to let us know it is sending us output associations. */
        OutputRule* orule = dynamic_cast<OutputRule *>(action->GetData());

        // if its ok, just add it to the list - we will send the first alarm
        // during simulation initialisation stage
        if (0 != orule)
        {
            m_outputRules.push_back(orule);
        }

        return 0;
    }

    ProcessTime_t OutputFileAlarm::SendNextOutputTime(ProcessActionDescriptor* action)
    {
        Use_Time;
        Use_PostOffice;
        ProcessTime_t next_time = time;
        ProcessTime_t next_closest_time = std::numeric_limits<ProcessTime_t>::max();

        for (OutputRuleCollection::iterator orule_iter = m_outputRules.begin();
            orule_iter != m_outputRules.end(); ++orule_iter)
        {
            next_time = (*orule_iter)->GetNextTime(time);
            if (next_time > time)
            {
                next_closest_time = Utility::CSMin(next_closest_time, next_time);
            }
        }

        if ((next_closest_time < std::numeric_limits<ProcessTime_t>::max()) && (next_closest_time > time))
        {
            Send_WakeUp_Message(next_closest_time);
        }

        return 0;
    }

    long int OutputFileAlarm::InvokeOutputRules(ProcessActionDescriptor* action)
    {
        Use_Time;
        Use_PostOffice;

        ProcessTime_t next_time = time;
        ProcessTime_t next_closest_time = std::numeric_limits<ProcessTime_t>::max();

        for (OutputRuleCollection::iterator orule_iter = m_outputRules.begin();
            orule_iter != m_outputRules.end(); ++orule_iter)
        {
            // 
            if ((*orule_iter)->DoesThisTime(time))
            {
                //action->SetData(*orule_iter);
                (*orule_iter)->ArrangeOutput(action);
            }

            next_time = (*orule_iter)->GetNextTime(time);
            if (next_time > time)
            {
                next_closest_time = Utility::CSMin(next_closest_time, next_time);
            }
        }

        if ((next_closest_time < std::numeric_limits<ProcessTime_t>::max()) && (next_closest_time > time))
        {
            Send_WakeUp_Message(next_closest_time);
        }

        return 0;
    }

    bool OutputFileAlarm::DoesOverride() const
    {
        return (true);
    }
} /* namespace rootmap */
