#ifndef SimulationEngine_H
#define SimulationEngine_H
/////////////////////////////////////////////////////////////////////////////
// Name:        SimulationEngine.h
// Purpose:     Declaration of the SimulationEngine class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/Types.h"
#include "core/log/Logger.h"
#include <list>
#include <typeinfo>

namespace rootmap
{
    // introducing, in alphabetical order...
    class DataAccessManager;
    class DataOutputCoordinator;
    class ISimulationActivityObserver;
    class PlantCoordinator;
    class PostOffice;
    class ProcessCoordinator;
    class RenderCoordinator;
    class ScoreboardCoordinator;
    class ScoreboardDataDAI;
    class SharedAttributeManager;
    class VolumeObjectCoordinator;

    // IMPORTANT NOTE: SimulationEngine 0wnz0rs the DataAccessManager pointer.
    // It's used as a reference in constructing a lot of SimulationEngine's members,
    // for historical reasons.

    class SimulationEngine
    {
    public:
        SimulationEngine(DataAccessManager& dam);
        virtual ~SimulationEngine();

        /**
         * accessor for the process coordinator instance
         *
         * @return a reference to the instance of said class
         */
        ProcessCoordinator& GetProcessCoordinator();

        /**
         * accessor for the scoreboard coordinator instance
         *
         * @return a reference to the instance of said class
         */
        ScoreboardCoordinator& GetScoreboardCoordinator();

        /**
         * accessor for the scoreboard coordinator instance
         *
         * @return a reference to the instance of said class
         */
        VolumeObjectCoordinator& GetVolumeObjectCoordinator();

        /**
         * accessor for the post office instance
         *
         * @return a reference to the instance of said class
         */
        PostOffice& GetPostOffice();

        /**
         * accessor for the post office instance,
           * for callers needing to access the post office's const functions
         *
         * @return a const reference to the instance of said class
         */
        const PostOffice& GetPostOffice() const;

        /**
         * accessor for the shared attribute manager instance
         *
         * @return a reference to the instance of said class
         */
        SharedAttributeManager& GetSharedAttributeManager();

        /**
         * accessor for the data access manager instance
         *
         * @return a reference to the instance of said class
         */
        DataAccessManager& GetDataAccessManager();

        /**
         * accessor for the data output coordinator instance
         *
         * @return a reference to the instance of said class
         */
        DataOutputCoordinator& GetDataOutputCoordinator();

        /**
         * Stage 3 of startup procedure : Initialisation.
         */
        virtual void Initialise();

        bool isInitialising() const;
        bool isInitialised() const;
        bool isStarting() const;
        bool isRunning() const;
        bool isPaused() const;
        bool isStopping() const;
        bool isIdling() const;
        bool IsBusy() const;

        /**
         *
         */
        virtual void Run();

        virtual void PauseToggle();

        /**
         *
         */
        virtual void Stop();

        /**
         *
         */
        virtual void RegisterSimulationActivityObserver(ISimulationActivityObserver* observer);

        /**
         *
         */
        virtual void RemoveSimulationActivityObserver(ISimulationActivityObserver* observer);

        /**
         * Called by DataAccessManagers
         */
        virtual void InitialiseScoreboardDataValues(ScoreboardDataDAI& data);

        virtual void SetCloseOnCompletion(const bool& b);

    protected:

        size_t GetNumSimulationActivityObservers() const;

        /**
         * Populate the scoreboards with the basic default values as specified
         * by Characteristic Defaults
         */
        virtual void InitialiseScoreboardCharacteristicDefaults();

        /**
         * Populate the scoreboards with values taken from sources nominated as
         * as Initialisation from configuration
         */
        virtual void InitialiseScoreboardCharacteristicData();

        /**
         *
         */
        virtual void InitialiseProcessData();

        /**
         *
         */
        virtual void DoInitialise();

        /**
         *
         */
        virtual void DoTerminate();

        /**
         * Don't override this function without good reason - use
         * ISimulationActivityObserver inheritance if possible
         */
        virtual void NotifyPreRun();
        /**
         * Don't override this function without good reason - use
         * ISimulationActivityObserver inheritance if possible
         */
        virtual void NotifyPreTimestamp();
        /**
         * Don't override this function without good reason - use
         * ISimulationActivityObserver inheritance if possible
         */
        virtual void NotifyMidTimestamp();
        /**
         * Don't override this function without good reason - use
         * ISimulationActivityObserver inheritance if possible
         */
        virtual void NotifyPostTimestamp();
        /**
         * Don't override this function without good reason - use
         * ISimulationActivityObserver inheritance if possible
         */
        virtual void NotifyPostRun();

        /**
         *    MSA 09.12.26 This function is called at the end of Run()
         *    if m_closeOnCompletion==true.
         *    It should be overridden by derived classes such as
         *    GuiSimulationEngine and used to destroy the SimulationEngine.
         *  Not yet implemented for CLI ROOTMAP.
         */
        virtual void CloseSimulation();

    private:
        RootMapLoggerDeclaration();

        ///
        ///
        SimulationPhase m_phase;

        // MSA 09.11.26 Implementing a close-on-completion feature.
        // May be modified in future to also support exit-on-completion,
        // for unattended runs and the like.
        // This would be particularly helpful for CLI ROOTMAP,
        // which could be scripted to perform multiple simulations in sequence,
        // along with associated file housekeeping where necessary,
        // and shut down the host computer when completed.
        bool m_closeOnCompletion;

        ///
        /// The process coordinator is one of the main objects that this engine
        /// class manages.
        ProcessCoordinator* m_processCoordinator;

        ///
        /// The scoreboard coordinator is another of the main objects that this
        /// engine class manages.
        ScoreboardCoordinator* m_scoreboardCoordinator;

        ///
        /// The volume object coordinator is another of the main objects that this
        /// engine class manages.
        VolumeObjectCoordinator* m_volumeObjectCoordinator;

        ///
        /// The post office is another of the main objects that this engine
        /// class manages.
        PostOffice* m_postOffice;


        ///
        ///
        PlantCoordinator* m_plantCoordinator;

        ///
        ///
        DataAccessManager& m_dataAccessManager;

        ///
        ///
        //DataPersistenceManager & m_dataPersistenceManager;

        ///
        ///
        DataOutputCoordinator* m_dataOutputCoordinator;

        ///
        ///    MSA 10.01.29 The RenderCoordinator
        /// is a main object which handles getting model information
        /// (root geometry, nutrient levels, etc)
        /// to raytracing OutputRules.
        RenderCoordinator* m_renderCoordinator;

        ///
        ///
        SharedAttributeManager* m_sharedAttributeManager;

        ///
        /// 
        typedef std::list<ISimulationActivityObserver *> SimulationActivityObserverCollection;

        ///
        /// hidden copy constructor and assignment operators
        SimulationEngine(const SimulationEngine&);
        SimulationEngine& operator=(const SimulationEngine&);

        ///
        ///
        // MSA 09.12.08 Making accessible to GuiSimulationEngine
    protected:
        SimulationActivityObserverCollection m_simulationActivityObservers;
    }; // class SimulationEngine

    inline bool SimulationEngine::isInitialising() const
    {
        return (simInitialising == m_phase);
    }

    inline bool SimulationEngine::isInitialised() const
    {
        return (simInitialised == m_phase);
    }

    inline bool SimulationEngine::isStarting() const
    {
        return (simStarting == m_phase);
    }

    inline bool SimulationEngine::isRunning() const
    {
        return (simRunning == m_phase || simPaused == m_phase || simPauseRequested == m_phase);
    }

    inline bool SimulationEngine::isPaused() const
    {
        return (simPaused == m_phase);
    }

    inline bool SimulationEngine::isStopping() const
    {
        return (simStopping == m_phase);
    }

    inline bool SimulationEngine::isIdling() const
    {
        return (simIdling == m_phase);
    }

    inline bool SimulationEngine::IsBusy() const
    {
        return ((m_phase >= simStarting) & (m_phase <= simStopping));
    }

    inline void SimulationEngine::SetCloseOnCompletion(const bool& b)
    {
        m_closeOnCompletion = b;
    }

    // inline SimulationPhase SimulationEngine::GetPhase() { return (m_phase); }
} /* namespace rootmap */

#endif // #ifndef SimulationEngine_H
