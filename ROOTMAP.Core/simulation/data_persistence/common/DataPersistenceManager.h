#ifndef DataPersistenceManager_H
#define DataPersistenceManager_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DataPersistenceManager.h
// Purpose:     Declaration of the DataPersistenceManager class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// The DataPersistenceManager is an abstract class that represents what is used by
// the SimulationEngine to form its simulation. The visitor pattern is used.
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Exceptions.h"

namespace rootmap
{
    class PersistedDataDirectory;
    class SharedAttributeManager;
    class ScoreboardCoordinator;
    class DataOutputCoordinator;
    class ProcessCoordinator;
    class SimulationEngine;
    class PlantCoordinator;
    class ITableDataMaintainer;
    class IViewDataMaintainer;
    class PostOffice;
    class Process;


    class DataPersistenceManager
    {
    public:
        // ////////////////////////////////////////////////////
        //
        // Construction
        //
        // ////////////////////////////////////////////////////
        /**
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        virtual void storeScoreboardConstruction(ScoreboardCoordinator* scoord) = 0;
        virtual void storeProcesseConstruction(ProcessCoordinator* pcoord) = 0;
        virtual void storePlantConstruction(PlantCoordinator* pcoord) = 0;
        virtual void storeOutputConstruction(DataOutputCoordinator* docoord) = 0;
        virtual void storePostOffice(PostOffice* poffice) = 0;
        virtual void storeSharedAttributeConstruction(SharedAttributeManager* sam) = 0;
        // void storeXXXXConstruction(XXXCoordinator * scoord) = 0;


        // ////////////////////////////////////////////////////
        //
        // Initialisation
        //
        // ////////////////////////////////////////////////////
        virtual void storeScoreboardData(SimulationEngine* sengine) = 0;

        /**
         * Whereas the constructOutputs is for information that requires no other
         * primary object (scoreboards, processes, postoffice) be stored,
         * this storage function is for the actual output rules, which require
         * the postoffice and destination output file alarm process.
         */
        virtual void storeOutputRules(DataOutputCoordinator* dcoord) = 0;

        /**
         * This function searches for process-special/specific data to store
         * the given process with
         */
        virtual void storeProcessSpecialData(Process* process) = 0;


        // ////////////////////////////////////////////////////
        //
        // Visualisation
        //
        // ////////////////////////////////////////////////////
        virtual void storeTableInformation(ITableDataMaintainer* maintainer) = 0;
        virtual void storeViewInformation(IViewDataMaintainer* maintainer) = 0;


        // ////////////////////////////////////////////////////
        //
        // Administration
        //
        // ////////////////////////////////////////////////////

        /**
         * Called by the SimulationEngine after all the construction,
         * initialisation and visualisation is complete. This is the time to
         * de-allocate memory, as it will no longer be used.
         *
         * The default implementation calls cleanup on the configuration store
         */
        virtual void cleanup();


        /**
         * Accessor for the datastore
         *
         * @return a reference to the datastore
         */
        PersistedDataDirectory& getPersistedDataDirectory();


    protected:
        DataPersistenceManager();
        virtual ~DataPersistenceManager();

        /**
         * Called by the derived DataManager class after the config data store
         * is instantiated.
         *
         * @param
         * @return
         */
        void setDataStore(PersistedDataDirectory* configurationDataStore);

    private:

        ///
        /// member declaration
        PersistedDataDirectory* m_configurationDataStore;
    }; // class DataPersistenceManager

    inline PersistedDataDirectory& DataPersistenceManager::getPersistedDataDirectory()
    {
        // RmAssert(m_configurationDataStore,"PersistedDataDirectory not yet set");
        //
        // but in the meantime, while RmAssert is being implemented,..
        if (m_configurationDataStore == 0)
        {
            throw RmException("PersistedDataDirectory not yet set");
        }
        return *m_configurationDataStore;
    }

    inline void DataPersistenceManager::setDataStore(PersistedDataDirectory* configurationDataStore)
    {
        m_configurationDataStore = configurationDataStore;
    }
} /* namespace rootmap */

#endif // #ifndef DataPersistenceManager_H
