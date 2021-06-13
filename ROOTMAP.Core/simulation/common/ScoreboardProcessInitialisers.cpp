/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardProcessInitialisers.cpp
// Purpose:     Implementation of the ScoreboardProcessDefaultInitialiser and
//              ScoreboardProcessDataInitialiser classes
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/ScoreboardProcessInitialisers.h"
#include "simulation/common/SimulationEngine.h"
#include "simulation/process/common/Process.h"
#include "simulation/process/common/ProcessExceptions.h"
#include "simulation/data_access/common/DataAccessManager.h"


namespace rootmap
{
    ScoreboardProcessDefaultInitialiser::ScoreboardProcessDefaultInitialiser(SimulationEngine& engine)
        : m_simulationEngine(engine)
        , m_currentScoreboard(__nullptr)
    {
        /* Do Nothing */
    }


    ScoreboardProcessDefaultInitialiser::~ScoreboardProcessDefaultInitialiser()
    {
        /* Do Nothing */
    }


    void ScoreboardProcessDefaultInitialiser::UseScoreboard(Scoreboard* scoreboard)
    {
        scoreboard->CalculateBoxVolumes();
        // for each scoreboard, iterate over processes, to reset values from there
        m_currentScoreboard = scoreboard;
        m_simulationEngine.GetProcessCoordinator().IterateOverProcesses(this, scoreboard->GetScoreboardStratum());
    }


    void ScoreboardProcessDefaultInitialiser::UseProcess(Process* /* process */)
    {
        /* Do Nothing */
    }


    void ScoreboardProcessDefaultInitialiser::UseProcess(Process* process, const ScoreboardStratum& /* stratum */)
    {
        // RmAssert(stratum == m_currentScoreboard->GetScoreboardStratum(), "Scoreboards do not match");
        process->InitialiseScoreboardDefaultValues(m_currentScoreboard);
    }


    RootMapLoggerDefinition(ScoreboardProcessDataInitialiser);


    ScoreboardProcessDataInitialiser::ScoreboardProcessDataInitialiser(SimulationEngine& engine)
        : m_simulationEngine(engine)
    {
        RootMapLoggerInitialisation("rootmap.ScoreboardProcessDataInitialiser");
    }


    ScoreboardProcessDataInitialiser::~ScoreboardProcessDataInitialiser()
    {
        /* Do Nothing */
    }

    void ScoreboardProcessDataInitialiser::UseProcess(Process* process)
    {
        try
        {
            m_simulationEngine.GetDataAccessManager().initialiseProcessData(process);
        }
        catch (ProcessException& processex)
        {
            LOG_ERROR << "Caught ProcessException while initialising Data for process " << processex.getProcessName() << ": " << processex.what();
        }
    }

    void ScoreboardProcessDataInitialiser::UseProcess(Process* /* process */, const ScoreboardStratum& /* stratum */)
    {
        /* Do Nothing */
    }
} /* namespace rootmap */
