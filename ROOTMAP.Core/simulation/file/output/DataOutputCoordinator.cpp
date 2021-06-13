/////////////////////////////////////////////////////////////////////////////
// Name:        DataOutputCoordinator.cpp
// Purpose:     Implementation of the DataOutputCoordinator class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/SimulationEngine.h"
#include "simulation/data_access/common/DataAccessManager.h"
#include "simulation/data_access/interface/OutputRuleDAI.h"
#include "simulation/file/output/DataOutputCoordinator.h"
#include "simulation/file/output/OutputFileAlarm.h"
#include "simulation/file/output/OutputRule.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/common/Process_Dictionary.h"
#include "simulation/process/interprocess_communication/PostOffice.h"


namespace rootmap
{
    RootMapLoggerDefinition(DataOutputCoordinator);

    DataOutputCoordinator::DataOutputCoordinator(SimulationEngine& engine, DataAccessManager& dam)
        : m_engine(engine)
    {
        RootMapLoggerInitialisation("rootmap.DataOutputCoordinator");
        // this actually does nothing right now - the output rules must come
        // about during initialisation, as they require the postoffice and 
        // OutputFileAlarm process
        dam.constructOutputs(this);
    }

    DataOutputCoordinator::~DataOutputCoordinator()
    {
    }

    void DataOutputCoordinator::InitialiseOutput(const OutputRuleDAI& data)
    {
        try
        {
            // RvH - OutputRules are owned by OutputFileAlarm
            OutputRule* new_rule = new OutputRule(data, m_engine);
            Process* output_alarm = m_engine.GetProcessCoordinator().FindProcessByProcessName("Output File Alarm");

            if (__nullptr != output_alarm)
            {
                m_engine.GetPostOffice().sendMessage(output_alarm, output_alarm, kExternalWakeUpMessage, new_rule);
            }
        }
        catch (RmException& rme)
        {
            LOG_ALERT << "Failed to use OutputRule: " << rme.what();
        }
    }

    const SimulationEngine& DataOutputCoordinator::GetSimulationEngine() const
    {
        return m_engine;
    }
} /* namespace rootmap */

