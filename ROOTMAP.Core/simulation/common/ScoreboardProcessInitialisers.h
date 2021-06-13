#ifndef ScoreboardProcessInitialisers_H
#define ScoreboardProcessInitialisers_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardProcessInitialisers.h
// Purpose:     Declaration of the ScoreboardProcessDefaultInitialiser and
//              ScoreboardProcessDataInitialiser classes
// Created:     06/06/2006
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
/////////////////////////////////////////////////////////////////////////////
#include "simulation/scoreboard/IScoreboardIterationUser.h"
#include "simulation/process/common/IProcessIterationUser.h"
#include "core/log/Logger.h"

namespace rootmap
{
    class Process;
    class Scoreboard;
    class ScoreboardStratum;
    class SimulationEngine;

    class ScoreboardProcessDefaultInitialiser
        : public IScoreboardIterationUser
        , public IProcessIterationUser
    {
    public:
        ScoreboardProcessDefaultInitialiser(SimulationEngine& engine);
        virtual ~ScoreboardProcessDefaultInitialiser();

        /**
         * UseScoreboard
         *
         * Called by the ScoreboardCoordinator for each scoreboard iteration.
         *
         * @param scoreboard - the current iteration of scoreboard to be used
         */
        virtual void UseScoreboard(Scoreboard* scoreboard);

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

        // hidden copy constructor
        ScoreboardProcessDefaultInitialiser(const ScoreboardProcessDefaultInitialiser&);

        ///
        ///
        SimulationEngine& m_simulationEngine;

        ///
        ///
        Scoreboard* m_currentScoreboard;
    }; // class ScoreboardProcessDefaultInitialiser

    class ScoreboardProcessDataInitialiser
        : public IProcessIterationUser
    {
    public:
        ScoreboardProcessDataInitialiser(SimulationEngine& engine);
        virtual ~ScoreboardProcessDataInitialiser();

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
        // hidden copy constructor
        ScoreboardProcessDataInitialiser(const ScoreboardProcessDataInitialiser&);

        RootMapLoggerDeclaration();

        ///
        ///
        SimulationEngine& m_simulationEngine;
    }; // class ScoreboardProcessDataInitialiser
} /* namespace rootmap */

#endif // #ifndef ScoreboardProcessInitialisers_H
